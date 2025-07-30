// sliders_commented.c

#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "psf_font.h"  // Підключи свої PSF-шрифти
extern PSF_Font font18;  // Шрифт для тексту
extern int spacing;     // Відступ між символами у тексті

#define SLIDER_KNOB_SIZE 12    // Розмір ручки слайдера
#define MAX_SLIDERS 10         // Максимальна кількість слайдерів одночасно

// Структура для збереження стану слайдера
typedef struct {
    Rectangle bounds;  // Позиція і розмір слайдера
    bool isActive;     // Чи активний слайдер (перетягується)
    bool used;         // Чи використовується цей слот
} SliderState;

// Масив станів слайдерів
static SliderState slidersState[MAX_SLIDERS] = {0};

// Індекс слайдера, що зараз перетягується (-1 якщо ні)
static int activeDraggingSlider = -1;

// Функція для пошуку або додавання слайдера за його bounds
static bool* GetSliderActiveState(Rectangle bounds) {
    for (int i = 0; i < MAX_SLIDERS; i++) {
        if (slidersState[i].used) {
            // Якщо слайдер з такими ж bounds вже є, повертаємо вказівник на його активність
            if (memcmp(&slidersState[i].bounds, &bounds, sizeof(Rectangle)) == 0) {
                return &slidersState[i].isActive;
            }
        } else {
            // Якщо слот вільний, ініціалізуємо його і повертаємо
            slidersState[i].bounds = bounds;
            slidersState[i].isActive = false;
            slidersState[i].used = true;
            return &slidersState[i].isActive;
        }
    }
    // Якщо всі слоти зайняті — повертаємо NULL
    return NULL;
}

// Функція для обчислення яскравості кольору (luminance)
static float GetLuminance(Color color) {
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Функція вибору контрастного кольору тексту (чорний або білий)
static Color GetContrastingTextColor(Color bgColor) {
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

// Функція обчислення довжини UTF-8 рядка (кількість символів)
/*
static int utf8_strlen(const char* s) {
    int len = 0;
    while (*s) {
        if ((*s & 0xc0) != 0x80) len++;
        s++;
    }
    return len;
}
*/

// Головна функція слайдера
// bounds - прямокутник слайдера
// font - шрифт для тексту
// textTop - підказка, що з'являється над слайдером при наведенні (може бути NULL)
// textRight - текст праворуч від слайдера (може бути NULL)
// value - вказівник на значення слайдера (float)
// minValue, maxValue - межі значення слайдера
// isVertical - true для вертикального слайдера, false для горизонтального
// baseColor - базовий колір слайдера
float Gui_Slider(Rectangle bounds, PSF_Font font, const char *textTop, const char *textRight,
                 float *value, float minValue, float maxValue, bool isVertical, Color baseColor) {
    // Отримуємо вказівник на стан активності слайдера
    bool *isActive = GetSliderActiveState(bounds);
    if (isActive == NULL) return *value; // Якщо не знайшли слот — повертаємо поточне значення

    Vector2 mousePos = GetMousePosition(); // Позиція миші
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds); // Чи курсор над слайдером

    // Нормалізоване значення слайдера від 0 до 1
    float normValue = (*value - minValue) / (maxValue - minValue);
    if (normValue < 0) normValue = 0;
    if (normValue > 1) normValue = 1;

    // Обробка натискань миші
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver && activeDraggingSlider == -1) {
        *isActive = true;
        // Визначаємо індекс активного слайдера
        for (int i = 0; i < MAX_SLIDERS; i++) {
            if (slidersState[i].used && memcmp(&slidersState[i].bounds, &bounds, sizeof(Rectangle)) == 0) {
                activeDraggingSlider = i;
                break;
            }
        }
    }
    // Обробка відпускання кнопки миші
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *isActive) {
        *isActive = false;
        activeDraggingSlider = -1;
    }

    // Якщо слайдер активний (перетягується) — оновлюємо значення
    if (*isActive && activeDraggingSlider != -1) {
        if (isVertical) {
            // Для вертикального слайдера інвертуємо координату, щоб верх = max
            normValue = 1.0f - (mousePos.y - bounds.y) / bounds.height;
        } else {
            normValue = (mousePos.x - bounds.x) / bounds.width;
        }
        if (normValue < 0) normValue = 0;
        if (normValue > 1) normValue = 1;

        *value = minValue + normValue * (maxValue - minValue);
    }

    // Колір фону слайдера з урахуванням активності і наведення
    Color sliderBgColor = baseColor;
    if (!(*isActive)) sliderBgColor = Fade(sliderBgColor, 0.4f);
    if (mouseOver) sliderBgColor = Fade(baseColor, 0.8f);

    // Контрастні кольори для рамки і тексту
    Color borderColor = GetContrastingTextColor(sliderBgColor);
    Color textColor = GetContrastingTextColor(borderColor);
    textColor.a = 255;  // Забезпечуємо повну непрозорість тексту

    // Малюємо фон слайдера
    DrawRectangleRec(bounds, sliderBgColor);

    // Малюємо рамку слайдера
    /* int borderThickness = 2;
    DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
                                     bounds.width + 2*borderThickness, bounds.height + 2*borderThickness},
                         borderThickness, borderColor); */

    // Малюємо подвійний контур слайдера
    // Товщина першої (внутрішньої) рамки
    float innerBorderThickness = 2.0f;
    // Товщина другої (зовнішньої) рамки
    float outerBorderThickness = 2.0f;

    // Колір внутрішньої рамки — контрастний до фону слайдера
    Color innerBorderColor = GetContrastingTextColor(sliderBgColor);

    // Колір зовнішньої рамки — інверсія внутрішньої (якщо чорний — білий, і навпаки)
    Color outerBorderColor = (innerBorderColor.r == 0 && innerBorderColor.g == 0 && innerBorderColor.b == 0) ? WHITE : BLACK;

    // Малюємо внутрішню рамку
    DrawRectangleLinesEx((Rectangle){ bounds.x - innerBorderThickness, bounds.y - innerBorderThickness,
                                    bounds.width + 2*innerBorderThickness, bounds.height + 2*innerBorderThickness },
                        innerBorderThickness, innerBorderColor);

    // Малюємо зовнішню рамку
    DrawRectangleLinesEx((Rectangle){ bounds.x - innerBorderThickness - outerBorderThickness, bounds.y - innerBorderThickness - outerBorderThickness,
                                    bounds.width + 2*(innerBorderThickness + outerBorderThickness), bounds.height + 2*(innerBorderThickness + outerBorderThickness) },
                        outerBorderThickness, outerBorderColor);

    // Малюємо повзунок (ручку)
    Color knobColor = *isActive ? GetContrastingTextColor(baseColor) : Fade(GetContrastingTextColor(baseColor), 0.5f);
    if (isVertical) {
        float knobY = bounds.y + (1.0f - normValue) * bounds.height;
        DrawRectangle(bounds.x, knobY - SLIDER_KNOB_SIZE/2, bounds.width, SLIDER_KNOB_SIZE, knobColor);
    } else {
        float knobX = bounds.x + normValue * bounds.width;
        DrawRectangle(knobX - SLIDER_KNOB_SIZE/2, bounds.y, SLIDER_KNOB_SIZE, bounds.height, knobColor);
    }

    // --- Малювання підказки textTop у контрастній рамці над слайдером ---
    if (mouseOver && textTop && textTop[0] != '\0') {
        int padding = 6;  // Відступи всередині рамки
        int charCount = utf8_strlen(textTop);
        float textWidth = charCount * (font.width + spacing) - spacing;
        float boxWidth = textWidth + 2 * padding;
        float boxHeight = font.height + 2 * padding;

        // Розміщуємо рамку по центру слайдера над ним
        Rectangle tooltipRect = {
            bounds.x + bounds.width / 2.0f - boxWidth / 2.0f,
            bounds.y - boxHeight - 8,
            boxWidth,
            boxHeight
        };

        // Малюємо фон і рамку підказки
        DrawRectangleRec(tooltipRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(tooltipRect, 1, textColor);

        // Малюємо текст підказки
        DrawPSFText(font,
                    tooltipRect.x + padding,
                    tooltipRect.y + padding / 2,
                    textTop, spacing, textColor);
    }

    // --- Малювання textRight у контрастній рамці праворуч від слайдера ---
    if (textRight && textRight[0] != '\0') {
        int padding = 6;
        int charCount = utf8_strlen(textRight);
        float textWidth = charCount * (font.width + spacing) - spacing;
        float boxWidth = textWidth + 2 * padding;
        float boxHeight = font.height + 2 * padding;

        // Рамка відцентрована по вертикалі біля правої межі слайдера
        Rectangle textRightRect = {
            bounds.x + bounds.width + 12,
            bounds.y + bounds.height / 2.0f - boxHeight / 2.0f,
            boxWidth,
            boxHeight
        };

        DrawRectangleRec(textRightRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(textRightRect, 1, textColor);

        DrawPSFText(font,
                    textRightRect.x + padding,
                    textRightRect.y + padding / 2,
                    textRight, spacing, textColor);
    }

    // --- Малювання значення слайдера у контрастній рамці праворуч, нижче textRight ---
    char valueText[16];
    snprintf(valueText, sizeof(valueText), "%.2f", *value);

    int paddingVal = 6;
    int charCountVal = utf8_strlen(valueText);
    float valTextWidth = charCountVal * (font.width + spacing) - spacing;
    float valBoxWidth = valTextWidth + 2 * paddingVal;
    float valBoxHeight = font.height + 2 * paddingVal;

    // Рамка для значення розміщується трохи нижче textRight
    Rectangle valueRect = {
        bounds.x + bounds.width + 12,
        bounds.y + bounds.height / 2.0f + 20,
        valBoxWidth,
        valBoxHeight
    };

    DrawRectangleRec(valueRect, baseColor);
    DrawRectangleLinesEx(valueRect, 1, GetContrastingTextColor(baseColor));

    DrawPSFText(font,
                valueRect.x + paddingVal,
                valueRect.y + paddingVal / 2,
                valueText, spacing, GetContrastingTextColor(baseColor));

    // Повертаємо оновлене значення слайдера
    return *value;
}

