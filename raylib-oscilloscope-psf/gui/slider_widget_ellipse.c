// slider_widget_ellipse.c
#include "slider_widget_ellipse.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Максимальна кількість слайдерів, які можна одночасно зареєструвати
#define MAX_SLIDERS 16

// Розмір ручки слайдера (в пікселях)
#define SLIDER_KNOB_SIZE 12

// Радіус області навколо ручки, в якій ловимось мишею (для зручності захоплення)
#define CAPTURE_RADIUS 10

// Структура, яка зберігає дані одного слайдера з еліпсною ручкою
typedef struct {
    Rectangle bounds;        // Прямокутна область слайдера на екрані
    float *value;            // Вказівник на змінну поточного значення слайдера
    float minValue;          // Мінімальне значення
    float maxValue;          // Максимальне значення
    bool isVertical;         // Орієнтація: true - вертикальний, false - горизонтальний
    Color baseColor;         // Базовий колір слайдера
    bool isActive;           // Чи захоплено ручку мишею (активний слайдер)
    bool used;               // Чи зайнято цей слот у масиві слайдерів
    const char* textTop;     // Текстова підказка над слайдером (може бути NULL)
    const char* textRight;   // Текстова підказка праворуч (може бути NULL)
} EllipseSlider;

// Статичний масив для зберігання усіх слайдерів
static EllipseSlider sliders[MAX_SLIDERS] = {0};

// Кількість зареєстрованих слайдерів (індекс першого вільного слота)
static int slidersCount = 0;

// Індекс активного слайдера (ручка якого захоплена мишею), -1, якщо відсутній
static int activeSliderIndex = -1;

// --- Зовнішня функція малювання тексту, реалізована поза цим файлом ---
// Використовується для малювання підказок та значень значень слайдера
extern void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, Color color);

/**
 * @brief Змінює насиченість кольору у HSV-просторі.
 *
 * @param c Колір у форматі Color (RGBA 0..255)
 * @param saturationScale Фактор зміни насиченості (>1 - яскравіше, <1 - тьмяніше)
 * @return Новий колір з модифікованою насиченістю
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
        if (cMax == r) {
            h = fmodf((g - b) / delta, 6.0f);
        } else if (cMax == g) {
            h = (b - r) / delta + 2.0f;
        } else {
            h = (r - g) / delta + 4.0f;
        }
        h *= 60.0f;
        if (h < 0) h += 360.0f;
    }

    float s = (cMax == 0) ? 0 : delta / cMax;
    float v = cMax;

    // Змінюємо насиченість
    s *= saturationScale;
    if (s > 1.0f) s = 1.0f;

    float cVal = v * s;
    float xVal = cVal * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - cVal;

    Color newColor = {0};

    // Конвертуємо назад у RGB
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
    newColor.a = c.a; // Альфа не міняємо
    return newColor;
}

/**
 * @brief Перевіряє чи миша знаходиться "біля" ручки слайдера (для зручності захоплення).
 *
 * @param mousePos Поточна позиція миші
 * @param slider Вказівник на слайдер
 * @return true якщо миша близько до ручки, false інакше
 */
static bool IsMouseNearKnob(Vector2 mousePos, EllipseSlider *slider) {
    if (!slider->used || slider->value == NULL) return false;

    // Обчислюємо нормалізоване значення (0..1)
    float normValue = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);
    float knobX, knobY;

    // Координати центру ручки в залежності від орієнтації
    if (slider->isVertical) {
        knobX = slider->bounds.x + slider->bounds.width / 2.0f;
        knobY = slider->bounds.y + (1.0f - normValue) * slider->bounds.height;
    } else {
        knobX = slider->bounds.x + normValue * slider->bounds.width;
        knobY = slider->bounds.y + slider->bounds.height / 2.0f;
    }

    float dx = mousePos.x - knobX;
    float dy = mousePos.y - knobY;
    float distSq = dx * dx + dy * dy;

    // Якщо відстань від миші до ручки менша ніж радіус захоплення, повертаємо true
    return (distSq <= CAPTURE_RADIUS * CAPTURE_RADIUS);
}

/**
 * @brief Оновлює значення слайдера на основі позиції миші.
 *
 * @param slider Вказівник на слайдер
 * @param mousePos Поточна позиція миші
 */
static void UpdateValueFromMouse(EllipseSlider *slider, Vector2 mousePos) {
    if (slider->value == NULL) return;

    float normValue;

    // Оновлюємо нормалізоване значення в залежності від орієнтації
    if (slider->isVertical) {
        normValue = 1.0f - (mousePos.y - slider->bounds.y) / slider->bounds.height;
    } else {
        normValue = (mousePos.x - slider->bounds.x) / slider->bounds.width;
    }

    // Клінуємо в [0..1]
    normValue = fmaxf(0.0f, fminf(1.0f, normValue));

    // Оновлюємо значення
    *slider->value = slider->minValue + normValue * (slider->maxValue - slider->minValue);
}

/**
 * @brief Визначає контрастний колір (чорний або білий) для кращої видимості.
 *
 * @param c Вхідний колір
 * @return Чорний або Білий колір, залежно від яскравості вхідного
 */
static Color GetContrastingColor(Color c) {
    float luminance = 0.2126f * c.r / 255.0f + 0.7152f * c.g / 255.0f + 0.0722f * c.b / 255.0f;
    return (luminance > 0.5f) ? BLACK : WHITE;
}

/**
 * @brief Реєструє новий слайдер з еліпсною ручкою або оновлює існуючий.
 *
 * @param sliderIndex Індекс слайдера у масиві (0..MAX_SLIDERS-1)
 * @param bounds Область розміщення слайдера
 * @param value Вказівник на змінну з поточним значенням слайдера
 * @param minValue Мінімальне значення слайдера
 * @param maxValue Максимальне значення слайдера
 * @param isVertical Орієнтація слайдера (true - вертикальний, false - горизонтальний)
 * @param baseColor Базовий колір для слайдера
 * @param textTop Текстова підказка над слайдером (може бути NULL)
 * @param textRight Текстова підказка праворуч від слайдера (може бути NULL)
 */
void RegisterEllipseKnobSlider(int sliderIndex, Rectangle bounds, float *value, float minValue, float maxValue,
                               bool isVertical, Color baseColor, const char* textTop, const char* textRight) {
    if (sliderIndex < 0 || sliderIndex >= MAX_SLIDERS) return;

    // Якщо слот пустий, налаштовуємо параметри
    if (!sliders[sliderIndex].used) {
        sliders[sliderIndex].bounds = bounds;
        sliders[sliderIndex].minValue = minValue;
        sliders[sliderIndex].maxValue = maxValue;
        sliders[sliderIndex].isVertical = isVertical;
        sliders[sliderIndex].baseColor = baseColor;
        sliders[sliderIndex].used = true;

        // Оновлюємо лічильник зареєстрованих слайдерів, якщо потрібно
        if (sliderIndex >= slidersCount)
            slidersCount = sliderIndex + 1;
    }

    // Оновлюємо вказівники та текстові поля
    sliders[sliderIndex].value = value;
    sliders[sliderIndex].textTop = textTop;
    sliders[sliderIndex].textRight = textRight;
}

/**
 * @brief Оновлює стан усіх слайдерів, обробляє взаємодію з мишею і малює їх.
 * Повинна викликатися один раз за кадр.
 *
 * @param font Шрифт для малювання тексту
 * @param spacing Відстань між символами шрифту
 */
void UpdateEllipseKnobSlidersAndDraw(PSF_Font font, int spacing) {
    Vector2 mousePos = GetMousePosition(); // Поточна позиція миші

    // Обробка початку перетягування ручки (натискання миші)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && activeSliderIndex == -1) {
        // Перебираємо слайдери у зворотньому порядку (щоб активувався верхній за малюванням)
        for (int i = slidersCount - 1; i >= 0; i--) {
            if (sliders[i].used && IsMouseNearKnob(mousePos, &sliders[i])) {
                // Деактивуємо всі слайдери
                for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
                // Активуємо цей слайдер
                activeSliderIndex = i;
                sliders[i].isActive = true;
                break;
            }
        }
    }
    // Обробка завершення перетягування (відпускання кнопки миші)
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
        activeSliderIndex = -1;
    }

    // Під час натискання миші оновлюємо значення активного слайдера відповідно до позиції миші
    if (activeSliderIndex != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        UpdateValueFromMouse(&sliders[activeSliderIndex], mousePos);
    }

    // Малюємо фон слайдерів, рамки та неактивні ручки
    for (int i = 0; i < slidersCount; i++) {
        EllipseSlider *slider = &sliders[i];
        if (!slider->used || slider->value == NULL) continue;

        // Нормалізоване значення (0..1) для позиції ручки
        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        // Малюємо напівпрозорий фон слайдера
        // DrawRectangleRec(slider->bounds, Fade(slider->baseColor, 0.20f));
        // Малюємо контрастну рамку
        // DrawRectangleLinesEx(slider->bounds, 1, GetContrastingColor(slider->baseColor));

        if (!slider->isActive) {
            // Колір ручки, може бути світлішим при наведенні
            Color knobColor = slider->baseColor;
            if (IsMouseNearKnob(mousePos, slider)) {
                knobColor = ChangeSaturation(slider->baseColor, 1.2f);
            }

            // Малюємо еліпс ручки в залежності від орієнтації
            if (slider->isVertical) {
                float knobY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
                float knobCenterX = slider->bounds.x + slider->bounds.width / 2.0f;
                float knobCenterY = knobY;
                float radiusX = slider->bounds.width / 2.0f;   // По ширині області слайдера / 2
                float radiusY = SLIDER_KNOB_SIZE / 2.0f;       // Заокруглення висоти ручки
                DrawEllipse(knobCenterX, knobCenterY, radiusX, radiusY, knobColor);
            } else {
                float knobX = slider->bounds.x + normVal * slider->bounds.width;
                float knobCenterX = knobX;
                float knobCenterY = slider->bounds.y + slider->bounds.height / 2.0f;
                float radiusX = SLIDER_KNOB_SIZE / 2.0f;       // Заокруглення ширини ручки
                float radiusY = slider->bounds.height / 2.0f;  // Висота області слайдера / 2
                DrawEllipse(knobCenterX, knobCenterY, radiusX, radiusY, knobColor);
            }
        }
    }

    // Малюємо активну ручку поверх усіх інших ручок
    if (activeSliderIndex != -1) {
        EllipseSlider *slider = &sliders[activeSliderIndex];
        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        // Колір активної ручки - яскравіший
        Color knobColor = ChangeSaturation(slider->baseColor, 1.5f);

        if (slider->isVertical) {
            float knobY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
            float knobCenterX = slider->bounds.x + slider->bounds.width / 2.0f;
            float knobCenterY = knobY;
            float radiusX = slider->bounds.width / 2.0f;
            float radiusY = SLIDER_KNOB_SIZE / 2.0f;
            DrawEllipse(knobCenterX, knobCenterY, radiusX, radiusY, knobColor);
        } else {
            float knobX = slider->bounds.x + normVal * slider->bounds.width;
            float knobCenterX = knobX;
            float knobCenterY = slider->bounds.y + slider->bounds.height / 2.0f;
            float radiusX = SLIDER_KNOB_SIZE / 2.0f;
            float radiusY = slider->bounds.height / 2.0f;
            DrawEllipse(knobCenterX, knobCenterY, radiusX, radiusY, knobColor);
        }
    }

    // Малюємо текстові підказки та числове значення для активного або наведеного слайдера
    for (int i = slidersCount - 1; i >= 0; i--) {
        EllipseSlider* slider = &sliders[i];
        if (!slider->used) continue;

        if (slider->isActive || IsMouseNearKnob(mousePos, slider)) {
            int padding = 6;

            // Текст над слайдером (textTop)
            if (slider->textTop && slider->textTop[0] != '\0') {
                int charCount = 0;
                const char *p = slider->textTop;
                // Підрахунок символів UTF-8
                while (*p) { if ((*p & 0xC0) != 0x80) charCount++; p++; }

                float textWidth = charCount * (font.width + spacing) - spacing;
                float boxWidth = textWidth + 2 * padding;
                float boxHeight = font.height + 2 * padding;

                // Область для підказки
                Rectangle tooltipRect = {
                    slider->bounds.x + slider->bounds.width / 2.0f - boxWidth / 2.0f,
                    slider->bounds.y - boxHeight - 8,
                    boxWidth,
                    boxHeight
                };

                Color bgColor = Fade(GetContrastingColor(slider->baseColor), 0.9f);
                Color textColor = GetContrastingColor(bgColor);

                // Малюємо фон та рамку підказки
                DrawRectangleRec(tooltipRect, bgColor);
                DrawRectangleLinesEx(tooltipRect, 1, textColor);

                // Виводимо текст підказки
                DrawPSFText(font, tooltipRect.x + padding, tooltipRect.y + padding / 2, slider->textTop, spacing, textColor);
            }

            // Текст праворуч від слайдера (textRight)
            if (slider->textRight && slider->textRight[0] != '\0') {
                int charCount = 0;
                const char *p = slider->textRight;
                while (*p) { if ((*p & 0xC0) != 0x80) charCount++; p++; }

                float textWidth = charCount * (font.width + spacing) - spacing;
                float boxWidth = textWidth + 2 * padding;
                float boxHeight = font.height + 2 * padding;

                // Область для тексту праворуч
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

                DrawPSFText(font, textRightRect.x + padding, textRightRect.y + padding / 2, slider->textRight, spacing, textColor);
            }

            // Малюємо числове значення слайдера
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

            // Область для значення
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

            // Виводимо значення слайдера
            DrawPSFText(font, valueRect.x + padding, valueRect.y + padding / 2, valueText, spacing, textColorVal);

            break; // Малюємо підказки тільки для одного слайдера (верхнього)
        }
    }
}

