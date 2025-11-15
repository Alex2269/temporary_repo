// slider_widget.c
// Віджет слайдерів з підтримкою множинних слайдерів,
// активною ручкою, підказками та контрастним малюванням.

#include "slider_widget.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Максимальна кількість слайдерів, які можна зареєструвати
#define MAX_SLIDERS 16

// Розмір ручки слайдера (в пікселях)
#define SLIDER_KNOB_SIZE 10

// Радіус захоплення мишею ручки слайдера (в пікселях)
#define CAPTURE_RADIUS 15

// Структура, що зберігає параметри одного слайдера
typedef struct {
    Rectangle bounds;        // Область слайдера на екрані
    float *value;            // Вказівник на значення слайдера
    float minValue;          // Мінімальне значення
    float maxValue;          // Максимальне значення
    bool isVertical;         // Орієнтація: true - вертикальний, false - горизонтальний
    Color baseColor;         // Базовий колір слайдера
    bool isActive;           // Чи активний слайдер (захоплений мишею)
    bool used;               // Чи використовується слот у масиві
    const char* textTop;     // Текст над слайдером
    const char* textRight;   // Текст праворуч від слайдера
} SliderEx;

// Масив для зберігання всіх слайдерів
static SliderEx sliders[MAX_SLIDERS] = {0};

// Кількість зареєстрованих слайдерів
static int slidersCount = 0;

// Індекс активного слайдера (-1, якщо немає)
static int activeSliderIndex = -1;

// Перевіряє, чи миша знаходиться поблизу ручки слайдера (в межах CAPTURE_RADIUS)
static bool IsMouseNearKnob(Vector2 mousePos, SliderEx *slider) {
    if (!slider->used || slider->value == NULL) return false;

    // Нормалізоване значення (0..1)
    float normValue = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);
    float knobX, knobY;

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

    return (distSq <= CAPTURE_RADIUS * CAPTURE_RADIUS);
}

// Оновлює значення слайдера на основі позиції миші
static void UpdateValueFromMouse(SliderEx *slider, Vector2 mousePos) {
    if (slider->value == NULL) return;

    float normValue;
    if (slider->isVertical) {
        normValue = 1.0f - (mousePos.y - slider->bounds.y) / slider->bounds.height;
    } else {
        normValue = (mousePos.x - slider->bounds.x) / slider->bounds.width;
    }
    normValue = fmaxf(0.0f, fminf(1.0f, normValue));
    *slider->value = slider->minValue + normValue * (slider->maxValue - slider->minValue);
}

// Реєструє слайдер або оновлює його параметри
void RegisterSlider(int sliderIndex, Rectangle bounds, float *value, float minValue, float maxValue, bool isVertical, Color baseColor, const char* textTop, const char* textRight) {
    if (sliderIndex < 0 || sliderIndex >= MAX_SLIDERS) return;

    if (!sliders[sliderIndex].used) {
        sliders[sliderIndex].bounds = bounds;
        sliders[sliderIndex].minValue = minValue;
        sliders[sliderIndex].maxValue = maxValue;
        sliders[sliderIndex].isVertical = isVertical;
        sliders[sliderIndex].baseColor = baseColor;
        sliders[sliderIndex].used = true;
        if (sliderIndex >= slidersCount) slidersCount = sliderIndex + 1;
    }
    sliders[sliderIndex].value = value;
    sliders[sliderIndex].textTop = textTop;
    sliders[sliderIndex].textRight = textRight;
}

// Основна функція оновлення та малювання слайдерів
void UpdateSlidersAndDraw(RasterFont font, int spacing) {
    Vector2 mousePos = GetMousePosition();

    // Обробка натискання миші: визначаємо активний слайдер
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && activeSliderIndex == -1) {
        for (int i = slidersCount - 1; i >= 0; i--) {
            if (sliders[i].used && IsMouseNearKnob(mousePos, &sliders[i])) {
                // Скидаємо активність у всіх слайдерів
                for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
                activeSliderIndex = i;
                sliders[i].isActive = true;
                break;
            }
        }
    }
    // Обробка відпускання кнопки миші: скидаємо активність
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        for (int k = 0; k < slidersCount; k++) sliders[k].isActive = false;
        activeSliderIndex = -1;
    }

    // Оновлення значення активного слайдера під час руху миші
    if (activeSliderIndex != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        UpdateValueFromMouse(&sliders[activeSliderIndex], mousePos);
    }

    // Малювання слайдерів (фони, рамки, неактивні ручки)
    for (int i = 0; i < slidersCount; i++) {
        SliderEx *slider = &sliders[i];
        if (!slider->used || slider->value == NULL) continue;

        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        // Малюємо фон та рамку слайдера
        DrawRectangleRec(slider->bounds, Fade(slider->baseColor, 0.20f));
        DrawRectangleLinesEx(slider->bounds, 1, GetContrastColor(slider->baseColor));

        // Малюємо ручку, якщо слайдер не активний
        if (!slider->isActive) {
            Color knobColor = slider->baseColor;
            if (IsMouseNearKnob(mousePos, slider)) {
                knobColor = ChangeSaturation(slider->baseColor, 1.2f);
            }
            if (slider->isVertical) {
                float knobY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
                DrawRectangle(slider->bounds.x, knobY - SLIDER_KNOB_SIZE / 2, slider->bounds.width, SLIDER_KNOB_SIZE, knobColor);
            } else {
                float knobX = slider->bounds.x + normVal * slider->bounds.width;
                DrawRectangle(knobX - SLIDER_KNOB_SIZE / 2, slider->bounds.y, SLIDER_KNOB_SIZE, slider->bounds.height, knobColor);
            }
        }
    }

    // Малювання активної ручки зверху (щоб не перекривалася іншими)
    if (activeSliderIndex != -1) {
        SliderEx *slider = &sliders[activeSliderIndex];
        float normVal = (*slider->value - slider->minValue) / (slider->maxValue - slider->minValue);

        Color knobColor = ChangeSaturation(slider->baseColor, 1.5f);

        if (slider->isVertical) {
            float knobY = slider->bounds.y + (1.0f - normVal) * slider->bounds.height;
            DrawRectangle(slider->bounds.x, knobY - SLIDER_KNOB_SIZE / 2, slider->bounds.width, SLIDER_KNOB_SIZE, knobColor);
        } else {
            float knobX = slider->bounds.x + normVal * slider->bounds.width;
            DrawRectangle(knobX - SLIDER_KNOB_SIZE / 2, slider->bounds.y, SLIDER_KNOB_SIZE, slider->bounds.height, knobColor);
        }
    }

    // Малювання текстів і значень для активного або наведенного слайдера
    for (int i = slidersCount - 1; i >= 0; i--) {
        SliderEx* slider = &sliders[i];
        if (!slider->used) continue;

        if (slider->isActive || IsMouseNearKnob(mousePos, slider)) {
            int padding = 6;

            // Малювання тексту зверху (textTop)
            if (slider->textTop && slider->textTop[0] != '\0') {
                int charCount = 0;
                const char *p = slider->textTop;
                while (*p) { if ((*p & 0xC0) != 0x80) charCount++; p++; }
                float textWidth = charCount * (font.glyph_width + spacing) - spacing;
                float boxWidth = textWidth + 2 * padding;
                float boxHeight = font.glyph_height + 2 * padding;

                Rectangle tooltipRect = {
                    slider->bounds.x + slider->bounds.width / 2.0f - boxWidth / 2.0f,
                    slider->bounds.y - boxHeight - 8,
                    boxWidth,
                    boxHeight
                };

                Color bgColor = Fade(GetContrastColor(slider->baseColor), 0.9f);
                Color textColor = GetContrastColor(bgColor);

                DrawRectangleRec(tooltipRect, bgColor);
                DrawRectangleLinesEx(tooltipRect, 1, textColor);
                DrawTextScaled(font, tooltipRect.x + padding, tooltipRect.y + padding / 2, slider->textTop, spacing, 1, textColor);
            }

            // Малювання тексту праворуч (textRight)
            if (slider->textRight && slider->textRight[0] != '\0') {
                int charCount = 0;
                const char *p = slider->textRight;
                while (*p) { if ((*p & 0xC0) != 0x80) charCount++; p++; }
                float textWidth = charCount * (font.glyph_width + spacing) - spacing;
                float boxWidth = textWidth + 2 * padding;
                float boxHeight = font.glyph_height + 2 * padding;

                Rectangle textRightRect = {
                    slider->bounds.x + slider->bounds.width + 12,
                    slider->bounds.y + slider->bounds.height / 2.0f - boxHeight / 2.0f,
                    boxWidth,
                    boxHeight
                };

                Color bgColor = Fade(GetContrastColor(slider->baseColor), 0.9f);
                Color textColor = GetContrastColor(bgColor);

                DrawRectangleRec(textRightRect, bgColor);
                DrawRectangleLinesEx(textRightRect, 1, textColor);
                DrawTextScaled(font, textRightRect.x + padding, textRightRect.y + padding / 2, slider->textRight, spacing, 1, textColor);
            }

            // Малювання числового значення слайдера
            char valueText[16];
            if (slider->value != NULL) {
                snprintf(valueText, sizeof(valueText), "%.2f", *slider->value);
            } else {
                snprintf(valueText, sizeof(valueText), "N/A");
            }

            int charCountVal = 0;
            const char *pv = valueText;
            while (*pv) { if ((*pv & 0xC0) != 0x80) charCountVal++; pv++; }
            float valTextWidth = charCountVal * (font.glyph_width+ spacing) - spacing;
            float valBoxWidth = valTextWidth + 2 * padding;
            float valBoxHeight = font.glyph_height + 2 * padding;

            Rectangle valueRect = {
                slider->bounds.x + slider->bounds.width + 12,
                slider->bounds.y + slider->bounds.height / 2.0f + 20,
                valBoxWidth,
                valBoxHeight
            };

            Color bgColorVal = Fade(slider->baseColor, 0.9f);
            Color textColorVal = GetContrastColor(bgColorVal);

            DrawRectangleRec(valueRect, bgColorVal);
            DrawRectangleLinesEx(valueRect, 1, textColorVal);
            DrawTextScaled(font, valueRect.x + padding, valueRect.y + padding / 2, valueText, spacing, 1, textColorVal);

            break; // Малюємо підказки лише для одного слайдера
        }
    }
}

