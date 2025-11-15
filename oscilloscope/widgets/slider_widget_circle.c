// slider_widget_circle.c

#include "slider_widget_circle.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Максимальна кількість слайдерів, які підтримуємо
#define MAX_SLIDERS 16

// Радіус круглої ручки в пікселях
#define SLIDER_KNOB_RADIUS 4

// Зона в пікселях навколо ручки, в якій вважаємо мишу "біля ручки"
#define CAPTURE_RADIUS 12

// Глобальні змінні-налаштування для відступів (як і в guicheckbox.c)
extern int LineSpacing;  // Відступ між рядками (пікселі)
extern int spacing;      // Відступ між символами (пікселі)

// Опис структури слайдера з круглою ручкою
typedef struct {
    Rectangle bounds;        // Позиція і розмір слайдера у вікні (в пікселях)
    float *value;            // Вказівник на значення (float), яке редагує слайдер
    float minValue;          // Мінімальне значення слайдера
    float maxValue;          // Максимальне значення слайдера
    bool isVertical;         // Орієнтація: true – вертикальний, false – горизонтальний
    Color baseColor;         // Основний колір елемента слайдера
    bool isActive;           // Чи захоплена ручка мишею (активний)
    bool used;               // Чи використовується слот слайдера
    const char* textTop;     // Текст над слайдером (підказка), може бути NULL
    const char* textRight;   // Текст праворуч від слайдера, може бути NULL
} CircleSlider;

// Масив для зберігання всіх слайдерів
static CircleSlider sliders[MAX_SLIDERS] = {0};
static int slidersCount = 0;         // Скільки слайдерів зареєстровано
static int activeSliderIndex = -1;   // Індекс слайдера, який зараз перетягують (-1 якщо нема)

// Перевірка, чи миша перебуває в зоні захоплення ручки слайдера
static bool IsMouseNearCircleKnob(Vector2 mousePos, CircleSlider *slider)
{
    if (!slider->used || slider->value == NULL) return false;

    // Обчислюємо нормалізоване значення слайдера в діапазоні [0..1]
    float normValue = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

    float knobX, knobY;
    if (slider->isVertical) {
        knobX = slider->bounds.x + slider->bounds.width / 2.0f;         // по центру ширини слайдера
        knobY = slider->bounds.y + (1.0f - normValue) * slider->bounds.height; // по висоті від низу до верху
    } else {
        knobX = slider->bounds.x + normValue * slider->bounds.width;    // по ширині від лівого краю
        knobY = slider->bounds.y + slider->bounds.height / 2.0f;        // по центру висоти
    }

    // Відстань від позиції миші до центру ручки
    float dx = mousePos.x - knobX;
    float dy = mousePos.y - knobY;

    // Порівнюємо квадрат відстані з квадратом радіуса захоплення
    return (dx*dx + dy*dy) <= (CAPTURE_RADIUS * CAPTURE_RADIUS);
}

// Оновлення значення слайдера згідно позиції миші
static void UpdateValueFromMouseCircle(CircleSlider *slider, Vector2 mousePos)
{
    if (slider->value == NULL) return;

    // Обчислення нормалізованої позиції [0..1] залежно від орієнтації
    float normValue;

    if (slider->isVertical) {
        normValue = 1.0f - (mousePos.y - slider->bounds.y) / slider->bounds.height;
    } else {
        normValue = (mousePos.x - slider->bounds.x) / slider->bounds.width;
    }

    // Обрізаємо, щоб не вийшло поза межі
    if (normValue < 0.0f) normValue = 0.0f;
    if (normValue > 1.0f) normValue = 1.0f;

    // Обчислюємо реальне значення слайдера відповідно до діапазону
    *slider->value = slider->minValue + normValue * (slider->maxValue - slider->minValue);
}

// Функція реєстрації або оновлення слайдера в масиві
void RegisterCircleKnobSlider(int sliderIndex, Rectangle bounds, float *value, float minValue, float maxValue,
                              bool isVertical, Color baseColor,
                              const char* textTop, const char* textRight)
{
    // Перевірка на вихід за межі масиву
    if (sliderIndex < 0 || sliderIndex >= MAX_SLIDERS) return;

    if (!sliders[sliderIndex].used) {
        // Заповнюємо основні параметри слайдера
        sliders[sliderIndex].bounds = bounds;
        sliders[sliderIndex].minValue = minValue;
        sliders[sliderIndex].maxValue = maxValue;
        sliders[sliderIndex].isVertical = isVertical;
        sliders[sliderIndex].baseColor = baseColor;
        sliders[sliderIndex].used = true;

        // Оновлюємо загальну кількість слайдерів, якщо індекс більший за поточний максимум
        if (sliderIndex >= slidersCount)
            slidersCount = sliderIndex + 1;
    }

    // Оновлюємо значення, тексти та стан активності
    sliders[sliderIndex].value = value;
    sliders[sliderIndex].textTop = textTop;
    sliders[sliderIndex].textRight = textRight;
    sliders[sliderIndex].isActive = false;
}

// Функція малювання круглої ручки слайдера
static void DrawCircleKnob(float centerX, float centerY, float radius, Color color)
{
    DrawCircle((int)centerX, (int)centerY, radius, color);
}

// Головна функція циклу оновлення і малювання слайдерів
void UpdateCircleKnobSlidersAndDraw(RasterFont font, int spacing)
{
    Vector2 mousePos = GetMousePosition();

    // Обробка початку перетягування: коли натиснули ліву кнопку миші і ніщо не активне
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && activeSliderIndex == -1)
    {
        // Перебираємо слайдери у зворотньому порядку, щоб зверху був активований верхній
        for (int i = slidersCount - 1; i >= 0; i--)
        {
            if (sliders[i].used && IsMouseNearCircleKnob(mousePos, &sliders[i]))
            {
                // Деактивуємо всі слайдери
                for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;

                // Активуємо слайдер, біля ручки якого миша
                activeSliderIndex = i;
                sliders[i].isActive = true;
                break;
            }
        }
    }
    // Обробка завершення перетягування - відпустили кнопку миші
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
        activeSliderIndex = -1;
    }

    // Якщо слайдер активний і кнопка миші тримається - оновлюємо значення
    if (activeSliderIndex != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        UpdateValueFromMouseCircle(&sliders[activeSliderIndex], mousePos);
    }

    // Локальні відступи для тексту
    int localSpacing = spacing;
    float lineSpacing = (float)LineSpacing;

    // Малюємо слайдери: фон, рамки і ручки
    for (int i = 0; i < slidersCount; i++)
    {
        CircleSlider *slider = &sliders[i];
        if (!slider->used || slider->value == NULL) continue;

        // Обчислюємо нормалізоване значення [0..1]
        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        // Малюємо напівпрозорий фон слайдера
        DrawRectangleRec(slider->bounds, Fade(slider->baseColor, 0.20f));
        // Малюємо контрастну рамку
        DrawRectangleLinesEx(slider->bounds, 1, GetContrastColor(slider->baseColor));

        Color knobColor = slider->baseColor;
        // Підсвічуємо ручку при наведенні миші, якщо вона не активна
        if (!slider->isActive && IsMouseNearCircleKnob(mousePos, slider)) {
            knobColor = ChangeSaturation(slider->baseColor, 1.2f);
        }

        // Розрахунок координат центру ручки залежно від орієнтації і значення
        float knobCenterX, knobCenterY;
        if (slider->isVertical) {
            knobCenterX = slider->bounds.x + slider->bounds.width / 2.0f;
            knobCenterY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
        } else {
            knobCenterX = slider->bounds.x + normVal * slider->bounds.width;
            knobCenterY = slider->bounds.y + slider->bounds.height / 2.0f;
        }

        // Малюємо круглу ручку
        DrawCircleKnob(knobCenterX, knobCenterY, SLIDER_KNOB_RADIUS, knobColor);
    }

    // Малюємо активну ручку поверх усіх інших (для виділення)
    if (activeSliderIndex != -1)
    {
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

    // Малюємо підказки textTop і textRight із підтримкою багаторядковості
    // Тільки для першого слайдера, який активний або навколо якого миша
    for (int i = slidersCount - 1; i >= 0; i--)
    {
        CircleSlider* slider = &sliders[i];
        if (!slider->used) continue;

        if (slider->isActive || IsMouseNearCircleKnob(mousePos, slider))
        {
            int padding = 6; // Відступи всередині блоків тексту

            // --- Малюємо текст над слайдером (textTop) ---
            if (slider->textTop && slider->textTop[0] != '\0')
            {
                // Розбиваємо текст на рядки по символу '\n' (максимум 10 рядків)
                const char* lines[10];
                int lineCountTop = 0;

                char tempText[256];
                strncpy(tempText, slider->textTop, sizeof(tempText) - 1);
                tempText[sizeof(tempText) - 1] = '\0'; // Гарантуємо null-термінування

                char* line = strtok(tempText, "\n");
                while (line != NULL && lineCountTop < 10)
                {
                    lines[lineCountTop++] = line;
                    line = strtok(NULL, "\n");
                }

                // Знаходимо максимальну ширину рядка в пікселях (рахунок символів UTF-8)
                float maxWidthTop = 0;
                for (int li = 0; li < lineCountTop; li++)
                {
                    int charCount = utf8_strlen(lines[li]);
                    float lineWidth = charCount * (font.glyph_width + localSpacing) - localSpacing;
                    if (lineWidth > maxWidthTop) maxWidthTop = lineWidth;
                }

                // Висота одного рядка тексту
                float lineHeightTop = (float)font.glyph_height;
                // Загальна висота блока тексту з урахуванням міжрядкових відступів і padding
                float totalHeightTop = lineCountTop * lineHeightTop + (lineCountTop - 1) * lineSpacing + 2 * padding;

                // Позиція і розмір прямокутника з фоном під текстом
                Rectangle tooltipRect = {
                    slider->bounds.x + slider->bounds.width / 2.0f - (maxWidthTop + 2 * padding) / 2.0f,
                    slider->bounds.y - totalHeightTop - 8, // 8 пікселів відступ зверху
                    maxWidthTop + 2 * padding,
                    totalHeightTop
                };

                // Малюємо напівпрозорий чорний фон для підказки
                DrawRectangleRec(tooltipRect, Fade(BLACK, 0.8f));
                // Малюємо білу контурну рамку
                DrawRectangleLinesEx(tooltipRect, 1, WHITE);

                // Пишемо кожний рядок тексту з вертикальним відступом між рядками
                for (int li = 0; li < lineCountTop; li++)
                {
                    DrawTextScaled(font,
                                (int)(tooltipRect.x + padding),
                                (int)(tooltipRect.y + padding / 2 + li * (lineHeightTop + lineSpacing)),
                                lines[li], localSpacing, 1, WHITE);
                }
            }

            // --- Малюємо текст праворуч від слайдера (textRight) ---
            if (slider->textRight && slider->textRight[0] != '\0')
            {
                // Аналогічно: розбиття на рядки по \n (максимум 10)
                const char* lines[10];
                int lineCountRight = 0;

                char tempText[256];
                strncpy(tempText, slider->textRight, sizeof(tempText) - 1);
                tempText[sizeof(tempText) - 1] = '\0';

                char* line = strtok(tempText, "\n");
                while (line != NULL && lineCountRight < 10)
                {
                    lines[lineCountRight++] = line;
                    line = strtok(NULL, "\n");
                }

                // Знаходимо максимальну ширину серед рядків
                float maxWidthRight = 0;
                for (int li = 0; li < lineCountRight; li++)
                {
                    int charCount = utf8_strlen(lines[li]);
                    float lineWidth = charCount * (font.glyph_width + localSpacing) - localSpacing;
                    if (lineWidth > maxWidthRight) maxWidthRight = lineWidth;
                }

                float lineHeightRight = (float)font.glyph_height;
                float totalHeightRight = lineCountRight * lineHeightRight + (lineCountRight - 1) * lineSpacing + 2 * padding;

                // Визначаємо позицію прямокутника для тексту праворуч, центровану по вертикалі
                Vector2 textRightPos = {
                    slider->bounds.x + slider->bounds.width + 10 + padding,
                    slider->bounds.y + (slider->bounds.height - totalHeightRight) / 2.0f + padding / 2.0f
                };

                Rectangle textRightBg = {
                    textRightPos.x - padding,
                    textRightPos.y - padding / 2.0f,
                    maxWidthRight + 2 * padding,
                    totalHeightRight
                };

                // Колір фону із прозорістю на основі базового кольору
                Color bgColor = Fade(slider->baseColor, 0.9f);
                Color borderColor = GetContrastColor(bgColor);
                Color textColor = borderColor;

                // Малюємо фон і рамку
                DrawRectangleRec(textRightBg, bgColor);
                DrawRectangleLinesEx(textRightBg, 1, borderColor);

                // Малюємо кожен рядок тексту
                for (int li = 0; li < lineCountRight; li++)
                {
                    DrawTextScaled(font,
                                (int)(textRightBg.x + padding),
                                (int)(textRightBg.y + padding / 2 + li * (lineHeightRight + lineSpacing)),
                                lines[li], localSpacing, 1, textColor);
                }
            }

            // --- Виводимо числове значення слайдера ---

            // Форматуємо значення в текст з двома знаками після коми
            char valueText[16];
            if (slider->value != NULL)
                snprintf(valueText, sizeof(valueText), "%.2f", *slider->value);
            else
                snprintf(valueText, sizeof(valueText), "N/A");

            // Підрахунок кількості символів (UTF-8)
            int charCountVal = 0;
            const char *pv = valueText;
            while (*pv) { if ((*pv & 0xC0) != 0x80) charCountVal++; pv++; }

            float valTextWidth = charCountVal * (font.glyph_width + localSpacing) - localSpacing;
            float valBoxWidth = valTextWidth + 2 * padding;
            float valBoxHeight = font.glyph_height + 2 * padding;

            // Позиція блоку з числовим значенням праворуч, під текстом
            Rectangle valueRect = {
                slider->bounds.x + slider->bounds.width + 12,
                slider->bounds.y + slider->bounds.height / 2.0f + 20,
                valBoxWidth,
                valBoxHeight
            };

            Color bgColorVal = Fade(slider->baseColor, 0.9f);
            Color textColorVal = GetContrastColor(bgColorVal);

            // Малюємо фон і рамку для числового значення
            DrawRectangleRec(valueRect, bgColorVal);
            DrawRectangleLinesEx(valueRect, 1, textColorVal);

            // Малюємо текст значення слайдера
            DrawTextScaled(font, (int)(valueRect.x + padding), (int)(valueRect.y + padding / 2), valueText, localSpacing, 1, textColorVal);

            // Показуємо підказки та значення лише для першого активного/наведеного слайдера
            break;
        }
    }
}

