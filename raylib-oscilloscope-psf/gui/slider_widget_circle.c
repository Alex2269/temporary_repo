// slider_widget_circle.c

#include "slider_widget_circle.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Максимальна кількість слайдерів, які можна одночасно зареєструвати
#define MAX_SLIDERS 16

// Радіус круглої ручки слайдера в пікселях
#define SLIDER_KNOB_RADIUS 5

// Радіус зони захоплення ручки (чутливість, у пікселях)
#define CAPTURE_RADIUS 12

/**
 * @brief Структура для слайдера з круглою ручкою.
 */
typedef struct {
    Rectangle bounds;        // Область слайдера на екрані (позиція та розмір)
    float *value;            // Вказівник на значення слайдера
    float minValue;          // Мінімальне допустиме значення слайдера
    float maxValue;          // Максимальне допустиме значення слайдера
    bool isVertical;         // Орієнтація слайдера (true - вертикальна, false - горизонтальна)
    Color baseColor;         // Основний колір слайдера
    bool isActive;           // Чи захоплена ручка мишею (активний слайдер)
    bool used;               // Прапорець, що вказує на зайнятість слоту
    const char* textTop;     // Текст над слайдером (може бути NULL)
    const char* textRight;   // Текст праворуч від слайдера (може бути NULL)
} CircleSlider;

// Масив слайдерів; індекси від 0 до MAX_SLIDERS-1
static CircleSlider sliders[MAX_SLIDERS] = {0};

// Поточна кількість зареєстрованих слайдерів
static int slidersCount = 0;

// Індекс активного слайдера (ручки, захопленої мишею), або -1, якщо відсутній
static int activeSliderIndex = -1;

/**
 * @brief Зовнішня функція для малювання тексту шрифтом PSF (реалізована поза цим файлом).
 *
 * @param font Шрифт PSF
 * @param x Координата X відносно вікна
 * @param y Координата Y відносно вікна
 * @param text Текст для відображення (рядок)
 * @param spacing Відстань між символами
 * @param color Колір тексту (RGBA)
 */
extern void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, Color color);

/**
 * @brief Змінює насиченість кольору, щоб виділити ручку при наведенні.
 *
 * Перетворює колір RGB у HSV, множить насиченість, зворотно конвертує.
 *
 * @param c Вхідний колір (RGBA)
 * @param saturationScale Множник насиченості (>1 – яскравіше)
 * @return Колір з відкоригованою насиченістю
 */
static Color ChangeSaturation(Color c, float saturationScale) {
    float r = c.r / 255.0f;
    float g = c.g / 255.0f;
    float b = c.b / 255.0f;

    float cMax = fmaxf(r, fmaxf(g, b));
    float cMin = fminf(r, fminf(g, b));
    float delta = cMax - cMin;

    float h = 0.0f;
    if (delta > 0) {
        if (cMax == r)
            h = fmodf((g - b) / delta, 6.0f);
        else if (cMax == g)
            h = (b - r) / delta + 2.0f;
        else
            h = (r - g) / delta + 4.0f;
        h *= 60.0f;
        if (h < 0) h += 360.0f;
    }

    float s = (cMax == 0) ? 0 : delta / cMax;
    float v = cMax;

    // Змінюємо насиченість із обмеженням [0,1]
    s *= saturationScale;
    if (s > 1.0f) s = 1.0f;

    float cVal = v * s;
    float xVal = cVal * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - cVal;

    Color newColor = {0};

    // Конвертуємо назад у RGB залежно від сектору кольору
    if (h >= 0 && h < 60) {
        newColor.r = (cVal + m) * 255;
        newColor.g = (xVal + m) * 255;
        newColor.b = (0 + m) * 255;
    } else if (h >= 60 && h < 120) {
        newColor.r = (xVal + m) * 255;
        newColor.g = (cVal + m) * 255;
        newColor.b = (0 + m) * 255;
    } else if (h >= 120 && h < 180) {
        newColor.r = (0 + m) * 255;
        newColor.g = (cVal + m) * 255;
        newColor.b = (xVal + m) * 255;
    } else if (h >= 180 && h < 240) {
        newColor.r = (0 + m) * 255;
        newColor.g = (xVal + m) * 255;
        newColor.b = (cVal + m) * 255;
    } else if (h >= 240 && h < 300) {
        newColor.r = (xVal + m) * 255;
        newColor.g = (0 + m) * 255;
        newColor.b = (cVal + m) * 255;
    } else if (h >= 300 && h < 360) {
        newColor.r = (cVal + m) * 255;
        newColor.g = (0 + m) * 255;
        newColor.b = (xVal + m) * 255;
    }
    newColor.a = c.a;
    return newColor;
}

/**
 * @brief Визначає контрастний колір (чорний або білий) для тексту на фоні.
 *
 * Обчислює яскравість кольору та повертає чорний або білий для хорошої читабельності.
 *
 * @param c Вхідний колір (RGBA)
 * @return Чорний (BLACK) або Білий (WHITE)
 */
static Color GetContrastingColor(Color c) {
    float luminance = 0.2126f * c.r / 255.0f + 0.7152f * c.g / 255.0f + 0.0722f * c.b / 255.0f;
    return (luminance > 0.5f) ? BLACK : WHITE;
}

/**
 * @brief Перевіряє, чи знаходиться курсор миші в зоні захоплення круглої ручки слайдера.
 *
 * @param mousePos Поточна позиція курсору миші
 * @param slider Вказівник на слайдер для перевірки
 * @return true, якщо курсор близький до ручки (зона CAPTURE_RADIUS), інакше false
 */
static bool IsMouseNearCircleKnob(Vector2 mousePos, CircleSlider *slider) {
    if (!slider->used || slider->value == NULL) return false;

    // Обчислюємо нормалізоване значення слайдера (0..1)
    float normValue = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

    float knobX, knobY;

    // Обчислення позиції центру ручки залежно від орієнтації слайдера
    if (slider->isVertical) {
        knobX = slider->bounds.x + slider->bounds.width / 2.0f;
        knobY = slider->bounds.y + (1.0f - normValue) * slider->bounds.height;
    } else {
        knobX = slider->bounds.x + normValue * slider->bounds.width;
        knobY = slider->bounds.y + slider->bounds.height / 2.0f;
    }

    float dx = mousePos.x - knobX;
    float dy = mousePos.y - knobY;

    // Межі радіусу захоплення
    float distSq = dx * dx + dy * dy;

    return distSq <= CAPTURE_RADIUS * CAPTURE_RADIUS;
}

/**
 * @brief Оновлює значення слайдера відповідно до позиції миші.
 *
 * Враховує орієнтацію слайдера (вертикальна або горизонтальна).
 * Значення нормалізується та обрізається в діапазон [minValue, maxValue].
 *
 * @param slider Вказівник на слайдер
 * @param mousePos Поточна позиція курсору миші
 */
static void UpdateValueFromMouseCircle(CircleSlider *slider, Vector2 mousePos) {
    if (slider->value == NULL) return;

    float normValue;

    if (slider->isVertical) {
        normValue = 1.0f - (mousePos.y - slider->bounds.y) / slider->bounds.height;
    } else {
        normValue = (mousePos.x - slider->bounds.x) / slider->bounds.width;
    }

    // Клінування нормалізованого значення в [0..1]
    normValue = fmaxf(0.0f, fminf(1.0f, normValue));

    *slider->value = slider->minValue + normValue * (slider->maxValue - slider->minValue);
}

/**
 * @brief Реєструє новий або оновлює існуючий слайдер з круглою ручкою.
 *
 * Індекс слайдера має бути в діапазоні [0, MAX_SLIDERS).
 * Якщо слот ще не зайнятий, налаштовує базові параметри.
 * Тексти textTop і textRight можуть бути NULL.
 *
 * @param sliderIndex Індекс слайдера в масиві (0..MAX_SLIDERS-1)
 * @param bounds Розташування і розмір слайдера на екрані
 * @param value Вказівник на значення слайдера (float)
 * @param minValue Мінімальне допустиме значення слайдера
 * @param maxValue Максимальне допустиме значення слайдера
 * @param isVertical Орієнтація слайдера (true - вертикальна, false - горизонтальна)
 * @param baseColor Основний колір слайдера
 * @param textTop Текст над слайдером (NULL – без тексту)
 * @param textRight Текст праворуч від слайдера (NULL – без тексту)
 */
void RegisterCircleKnobSlider(int sliderIndex, Rectangle bounds, float *value, float minValue, float maxValue,
                              bool isVertical, Color baseColor,
                              const char* textTop, const char* textRight) {
    if (sliderIndex < 0 || sliderIndex >= MAX_SLIDERS) return;

    // Якщо слот не зайнятий, ініціалізуємо параметри слайдера
    if (!sliders[sliderIndex].used) {
        sliders[sliderIndex].bounds = bounds;
        sliders[sliderIndex].minValue = minValue;
        sliders[sliderIndex].maxValue = maxValue;
        sliders[sliderIndex].isVertical = isVertical;
        sliders[sliderIndex].baseColor = baseColor;
        sliders[sliderIndex].used = true;

        // Оновлення лічильника зареєстрованих слайдерів за потребою
        if (sliderIndex >= slidersCount)
            slidersCount = sliderIndex + 1;
    }

    // Завжди оновлюємо вказівник на значення та текстові підказки
    sliders[sliderIndex].value = value;
    sliders[sliderIndex].textTop = textTop;
    sliders[sliderIndex].textRight = textRight;
}

/**
 * @brief Малює круглу ручку слайдера.
 *
 * @param centerX Координата X центру ручки в пікселях
 * @param centerY Координата Y центру ручки в пікселях
 * @param radius Радіус ручки в пікселях
 * @param color Колір заливки (RGBA)
 */
static void DrawCircleKnob(float centerX, float centerY, float radius, Color color) {
    DrawCircle((int)centerX, (int)centerY, radius, color);
}

/**
 * @brief Оновлює стан слайдерів, обробляє введення та малює їх.
 *
 * Викликати один раз за кадр в циклі малювання.
 * Обробляє:
 * - Захоплення і відпускання миші для перетягування ручок.
 * - Оновлення значень слайдерів за позицією миші.
 * - Малювання фонів, ручок, підказок, числових значень.
 *
 * @param font Шрифт PSF для виводу тексту
 * @param spacing Відстань між символами при малюванні тексту
 */
void UpdateCircleKnobSlidersAndDraw(PSF_Font font, int spacing) {
    Vector2 mousePos = GetMousePosition();

    // Початок перетягування ручки (натискання лівої кнопки миші)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && activeSliderIndex == -1) {
        // Перебираємо слайдери у зворотньому порядку, щоб активувати верхній за малюванням
        for (int i = slidersCount - 1; i >= 0; i--) {
            if (sliders[i].used && IsMouseNearCircleKnob(mousePos, &sliders[i])) {
                // Деактивуємо всі слайдери
                for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
                // Активуємо обраний слайдер
                activeSliderIndex = i;
                sliders[i].isActive = true;
                break;
            }
        }
    }
    // Завершення перетягування (відпускання кнопки миші)
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
        activeSliderIndex = -1;
    }

    // Оновлення значення активного слайдера під час натискання миші
    if (activeSliderIndex != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        UpdateValueFromMouseCircle(&sliders[activeSliderIndex], mousePos);
    }

    // Малюємо фони, рамки і неактивні ручки для всіх слайдерів
    for (int i = 0; i < slidersCount; i++) {
        CircleSlider *slider = &sliders[i];

        if (!slider->used || slider->value == NULL) continue;

        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        // Малюємо напівпрозорий фон слайдера
        // DrawRectangleRec(slider->bounds, Fade(slider->baseColor, 0.20f));
        // Малюємо контрастну рамку навколо слайдера
        // DrawRectangleLinesEx(slider->bounds, 1, GetContrastingColor(slider->baseColor));

        Color knobColor = slider->baseColor;

        // Якщо ручка не активна, підсвічуємо при наведенні курсору
        if (!slider->isActive && IsMouseNearCircleKnob(mousePos, slider)) {
                knobColor = ChangeSaturation(slider->baseColor, 1.2f);
            }

        float knobCenterX, knobCenterY;

        if (slider->isVertical) {
            knobCenterX = slider->bounds.x + slider->bounds.width / 2.0f;
            knobCenterY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
        } else {
            knobCenterX = slider->bounds.x + normVal * slider->bounds.width;
            knobCenterY = slider->bounds.y + slider->bounds.height / 2.0f;
        }

        DrawCircleKnob(knobCenterX, knobCenterY, SLIDER_KNOB_RADIUS, knobColor);
    }

    // Малюємо активну ручку поверх усіх інших для виділення
    if (activeSliderIndex != -1) {
        CircleSlider *slider = &sliders[activeSliderIndex];
        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        Color knobColor = ChangeSaturation(slider->baseColor, 1.5f);

        float knobCenterX, knobCenterY;

        if (slider->isVertical) {
            knobCenterX = slider->bounds.x + slider->bounds.width / 2.0f;
            knobCenterY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
        } else {
            knobCenterX = slider->bounds.x + normVal * slider->bounds.width;
            knobCenterY = slider->bounds.y + slider->bounds.height / 2.0f;
        }

        DrawCircleKnob(knobCenterX, knobCenterY, SLIDER_KNOB_RADIUS, knobColor);
    }

    // Малювання підказок і числового значення для активного або наведеного слайдера
    for (int i = slidersCount - 1; i >= 0; i--) {
        CircleSlider* slider = &sliders[i];
        if (!slider->used) continue;

        if (slider->isActive || IsMouseNearCircleKnob(mousePos, slider)) {
            int padding = 6;

            // Підрахунок кількості символів у UTF-8 для ширини тексту
                int charCount = 0;
            const char *p;

            // Вивід тексту над слайдером, якщо є
            if (slider->textTop && slider->textTop[0] != '\0') {
                charCount = 0;
                p = slider->textTop;
                while (*p) { if ((*p & 0xC0) != 0x80) charCount++; p++; }

                float textWidth = charCount * (font.width + spacing) - spacing;
                float boxWidth = textWidth + 2 * padding;
                float boxHeight = font.height + 2 * padding;

                // Області для підказки зверху
                Rectangle tooltipRect = {
                    slider->bounds.x + slider->bounds.width / 2.0f - boxWidth / 2.0f,
                    slider->bounds.y - boxHeight - 8,
                    boxWidth,
                    boxHeight
                };

                Color bgColor = Fade(GetContrastingColor(slider->baseColor), 0.9f);
                Color textColor = GetContrastingColor(bgColor);

                DrawRectangleRec(tooltipRect, bgColor);
                DrawRectangleLinesEx(tooltipRect, 1, textColor);
                DrawPSFText(font, (int)(tooltipRect.x + padding), (int)(tooltipRect.y + padding / 2), slider->textTop, spacing, textColor);
            }

            // Вивід тексту праворуч від слайдера, якщо встановлений
            if (slider->textRight && slider->textRight[0] != '\0') {
                charCount = 0;
                p = slider->textRight;
                while (*p) { if ((*p & 0xC0) != 0x80) charCount++; p++; }

                float textWidth = charCount * (font.width + spacing) - spacing;
                float boxWidth = textWidth + 2 * padding;
                float boxHeight = font.height + 2 * padding;

                Rectangle textRightRect = {
                    slider->bounds.x + slider->bounds.width + 12,
                    slider->bounds.y + slider->bounds.height / 2.0f - boxHeight / 2.0f,
                    boxWidth,
                    boxHeight
                };

                Color bgColor = Fade(GetContrastingColor(slider->baseColor), 0.9f);
                Color textColor = GetContrastingColor(bgColor);

                DrawRectangleRec(textRightRect, bgColor);
                DrawRectangleLinesEx(textRightRect, 1, textColor);
                DrawPSFText(font, (int)(textRightRect.x + padding), (int)(textRightRect.y + padding / 2), slider->textRight, spacing, textColor);
            }

            // Відображення числового значення слайдера
            char valueText[16];
            if (slider->value != NULL) {
                snprintf(valueText, sizeof(valueText), "%.2f", *slider->value);
            } else {
                snprintf(valueText, sizeof(valueText), "N/A");
            }

            int charCountVal = 0;
            const char *pv = valueText;
            while (*pv) { if ((*pv & 0xC0) != 0x80) charCountVal++; pv++; }

            float valTextWidth = charCountVal * (font.width + spacing) - spacing;
            float valBoxWidth = valTextWidth + 2 * padding;
            float valBoxHeight = font.height + 2 * padding;

            Rectangle valueRect = {
                slider->bounds.x + slider->bounds.width + 12,
                slider->bounds.y + slider->bounds.height / 2.0f + 20,
                valBoxWidth,
                valBoxHeight
            };

            Color bgColorVal = Fade(slider->baseColor, 0.9f);
            Color textColorVal = GetContrastingColor(bgColorVal);

            DrawRectangleRec(valueRect, bgColorVal);
            DrawRectangleLinesEx(valueRect, 1, textColorVal);
            DrawPSFText(font, (int)(valueRect.x + padding), (int)(valueRect.y + padding / 2), valueText, spacing, textColorVal);

            // Показуємо підказки і значення лише для першого підходящого слайдера
            break;
        }
    }
}

