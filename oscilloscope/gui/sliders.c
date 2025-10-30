// sliders.c

#include "raylib.h"
#include "sliders.h"
#include <stdio.h>
#include <string.h>

extern int spacing;      // Відступ між символами у тексті
extern int LineSpacing;  // Відступ між рядками тексту

#define SLIDER_KNOB_SIZE 12    // Розмір ручки слайдера (ширина або висота залежно від орієнтації)
#define MAX_SLIDERS 10         // Максимальна кількість слайдерів, які можуть існувати одночасно

// Структура, що зберігає стан слайдера
typedef struct {
    Rectangle bounds;  // Прямокутна область слайдера (позиція + розмір)
    bool isActive;     // Чи слайдер зараз перетягується
    bool used;         // Чи цей слот використовується
} SliderState;

// Масив збереження станів слайдерів
static SliderState slidersState[MAX_SLIDERS] = {0};

// Індекс слайдера, який наразі перетягують (-1 означає, що активних немає)
static int activeDraggingSlider = -1;

// Функція пошуку або ініціалізації стану слайдера по його bounds
// Повертає вказівник на поле isActive або NULL, якщо немає вільних слотів
static bool* GetSliderActiveState(Rectangle bounds) {
    for (int i = 0; i < MAX_SLIDERS; i++) {
        if (slidersState[i].used) {
            // Порівнюємо bounds слайдера в масиві з переданими bounds
            if (memcmp(&slidersState[i].bounds, &bounds, sizeof(Rectangle)) == 0) {
                return &slidersState[i].isActive;
            }
        } else {
            // Якщо цей слот не використовується, ініціалізуємо його та повертаємо вказівник на isActive
            slidersState[i].bounds = bounds;
            slidersState[i].isActive = false;
            slidersState[i].used = true;
            return &slidersState[i].isActive;
        }
    }
    // Коли всі слоти зайняті, повертаємо NULL
    return NULL;
}

// Функція обчислення яскравості кольору для вибору контрастного кольору тексту
static float GetLuminance(Color color) {
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    // Формула лумінансу на основі сприйняття людського ока
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Функція повертає чорний або білий колір залежно від яскравості фону
static Color GetContrastingTextColor(Color bgColor) {
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

// Підрахунок кількості символів UTF-8 в рядку (не байтів)
/*
static int utf8_strlen(const char* s) {
    int len = 0;
    while (*s) {
        // Якщо байт не є продовженням UTF-8 (маска 0xC0 != 0x80), збільшуємо лічильник символів
        if ((*s & 0xc0) != 0x80) len++;
        s++;
    }
    return len;
} */

// Розбиття тексту по '\n' на масив рядків, повертає кількість рядків
// Зверніть увагу: модифікує вхідний текст (використовує strtok)
static int SplitTextLines(char *text, const char **lines, int maxLines) {
    int count = 0;
    char *line = strtok(text, "\n");
    while (line != NULL && count < maxLines) {
        lines[count++] = line;
        line = strtok(NULL, "\n");
    }
    return count;
}

// Основна функція слайдера
// Відповідає за відображення, керування і відображення підказок та значення
float Gui_Slider(Rectangle bounds, RasterFont font, const char *textTop, const char *textRight,
                 float *value, float minValue, float maxValue, bool isVertical, Color baseColor) {
    // Отримуємо вказівник на поле isActive конкретного слайдера за його bounds
    bool *isActive = GetSliderActiveState(bounds);
    if (isActive == NULL) return *value; // Якщо штатних слотів немає - просто повертаємо поточне значення

    Vector2 mousePos = GetMousePosition();  // Позиція миші на екрані
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds); // Чи знаходиться курсор над слайдером?

    // Нормалізуємо значення слайдера в діапазон 0..1
    float normValue = (*value - minValue) / (maxValue - minValue);
    normValue = normValue < 0 ? 0 : (normValue > 1 ? 1 : normValue); // clamp 0..1

    // Обчислюємо положення ручки слайдера (knob)
    Rectangle knobRect;
    if (isVertical) {
        // Для вертикального слайдера ручка рухається по осі Y (зверху вниз)
        float knobY = bounds.y + (1.0f - normValue) * bounds.height;
        knobRect = (Rectangle){ bounds.x, knobY - SLIDER_KNOB_SIZE / 2, bounds.width, SLIDER_KNOB_SIZE };
    } else {
        // Для горизонтального слайдера ручка рухається по осі X (зліва направо)
        float knobX = bounds.x + normValue * bounds.width;
        knobRect = (Rectangle){ knobX - SLIDER_KNOB_SIZE / 2, bounds.y, SLIDER_KNOB_SIZE, bounds.height };
    }

    // Розширена зона "липкості" (sticky area), щоб зручніше було взаємодіяти
    const int stickyMargin = 10;
    Rectangle stickyRect = knobRect;
    if (isVertical) {
        stickyRect.y -= stickyMargin;
        stickyRect.height += 2 * stickyMargin;
    } else {
        stickyRect.x -= stickyMargin;
        stickyRect.width += 2 * stickyMargin;
    }

    // Перевіряємо, чи курсор знаходиться в зоні прилипання ручки
    bool mouseOverSticky = CheckCollisionPointRec(mousePos, stickyRect);

    // Починаємо перетягувати слайдер, якщо натиснули мишу в зоні прилипання і ніхто інший не перетягує
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOverSticky && activeDraggingSlider == -1) {
        *isActive = true;
        // Визначаємо індекс слайдера в масиві для подальшого управління
        for (int i = 0; i < MAX_SLIDERS; i++) {
            if (slidersState[i].used && memcmp(&slidersState[i].bounds, &bounds, sizeof(Rectangle)) == 0) {
                activeDraggingSlider = i;
                break;
            }
        }
    }

    // Завершуємо перетягування при відпусканні кнопки миші
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *isActive) {
        *isActive = false;
        activeDraggingSlider = -1;
    }

    // Якщо слайдер зараз перетягується, оновлюємо normValue відповідно до курсору
    if (*isActive && activeDraggingSlider != -1) {
        if (isVertical) {
            normValue = 1.0f - (mousePos.y - bounds.y) / bounds.height;
        } else {
            normValue = (mousePos.x - bounds.x) / bounds.width;
        }
        normValue = normValue < 0 ? 0 : (normValue > 1 ? 1 : normValue);
        *value = minValue + normValue * (maxValue - minValue);
    }

    // Підбір кольору фону, залежно від активності та наведення миші
    Color sliderBgColor = baseColor;
    if (!*isActive) sliderBgColor = Fade(sliderBgColor, 0.4f); // затемнити, якщо не активний
    if (mouseOver) sliderBgColor = Fade(baseColor, 0.8f);     // легке підсвічування при наведенні

    // Визначення кольорів рамки і тексту, щоб був достатній контраст
    Color borderColor = GetContrastingTextColor(sliderBgColor);
    Color textColor = GetContrastingTextColor(borderColor);
    textColor.a = 255; // повна непрозорість тексту

    // Малюємо фон слайдера
    DrawRectangleRec(bounds, sliderBgColor);

    // Малюємо подвійний контур для кращої видимості
    float innerBorderThickness = 2.0f;
    float outerBorderThickness = 2.0f;

    Color innerBorderColor = borderColor;
    Color outerBorderColor = (innerBorderColor.r == 0 && innerBorderColor.g == 0 && innerBorderColor.b == 0) ? WHITE : BLACK;

    DrawRectangleLinesEx((Rectangle){
        bounds.x - innerBorderThickness, bounds.y - innerBorderThickness,
        bounds.width + 2 * innerBorderThickness, bounds.height + 2 * innerBorderThickness
    }, innerBorderThickness, innerBorderColor);

    DrawRectangleLinesEx((Rectangle){
        bounds.x - innerBorderThickness - outerBorderThickness, bounds.y - innerBorderThickness - outerBorderThickness,
        bounds.width + 2 * (innerBorderThickness + outerBorderThickness), bounds.height + 2 * (innerBorderThickness + outerBorderThickness)
    }, outerBorderThickness, outerBorderColor);

    // Малюємо ручку (knob) слайдера
    Color knobColor = *isActive ? GetContrastingTextColor(baseColor) : Fade(GetContrastingTextColor(baseColor), 0.5f);
    if (isVertical) {
        float knobY = bounds.y + (1.0f - normValue) * bounds.height;
        DrawRectangle(bounds.x, knobY - SLIDER_KNOB_SIZE / 2, bounds.width, SLIDER_KNOB_SIZE, knobColor);
    } else {
        float knobX = bounds.x + normValue * bounds.width;
        DrawRectangle(knobX - SLIDER_KNOB_SIZE / 2, bounds.y, SLIDER_KNOB_SIZE, bounds.height, knobColor);
    }

    // --- Відображення багаторядкового тексту textTop (підказка над слайдером) ---
    // Відображаємо тільки, якщо миша наведена і текст не порожній
    if (mouseOver && textTop && textTop[0] != '\0') {
        int padding = 6; // Внутрішній відступ рамки підказки

        // Копіюємо текст у тимчасовий буфер для розбиття по рядках
        char tempText[256];
        strncpy(tempText, textTop, sizeof(tempText) - 1);
        tempText[sizeof(tempText) - 1] = '\0';

        // Масив рядків з максимумом 10
        const char *lines[10];
        int lineCount = SplitTextLines(tempText, lines, 10);

        // Обчислюємо максимальну ширину рядків для коректного розміру мультилиста
        float maxWidth = 0;
        for (int i = 0; i < lineCount; i++) {
            int charCount = utf8_strlen(lines[i]);
            float lineWidth = charCount * (font.glyph_width + spacing) - spacing;
            if (lineWidth > maxWidth) maxWidth = lineWidth;
        }

        // Висота усіх рядків з урахуванням міжрядкового інтервалу і падінгів
        float lineHeight = (float)font.glyph_height;
        float totalHeight = lineCount * lineHeight + (lineCount - 1) * LineSpacing + 2 * padding;

        // Прямокутник виводу підказки центрований по ширині слайдера і над ним
        Rectangle tooltipRect = {
            bounds.x + bounds.width / 2.0f - (maxWidth + 2 * padding) / 2.0f,
            bounds.y - totalHeight - 8, // Невеликий відступ зверху
            maxWidth + 2 * padding,
            totalHeight
        };

        // Малюємо фон рамки та її контур
        DrawRectangleRec(tooltipRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(tooltipRect, 1, textColor);

        // Виводимо кожен рядок тексту з урахуванням висоти рядка і відступів
        for (int i = 0; i < lineCount; i++) {
            DrawTextScaled(font,
                        (int)(tooltipRect.x + padding),
                        (int)(tooltipRect.y + padding / 2 + i * (lineHeight + LineSpacing)),
                        lines[i], spacing, 1, textColor);
        }
    }

    // --- Відображення багаторядкового тексту textRight праворуч від слайдера ---
    // Відображати лише при наведенні миші та якщо текст не пустий
    if (mouseOver && textRight && textRight[0] != '\0') {
        int padding = 6;

        char tempText[256];
        strncpy(tempText, textRight, sizeof(tempText) - 1);
        tempText[sizeof(tempText) - 1] = '\0';

        const char *lines[10];
        int lineCount = SplitTextLines(tempText, lines, 10);

        float maxWidth = 0;
        for (int i = 0; i < lineCount; i++) {
            int charCount = utf8_strlen(lines[i]);
            float lineWidth = charCount * (font.glyph_width + spacing) - spacing;
            if (lineWidth > maxWidth) maxWidth = lineWidth;
        }

        float lineHeight = (float)font.glyph_height;
        float totalHeight = lineCount * lineHeight + (lineCount - 1) * LineSpacing + 2 * padding;

        // Рамка праворуч від слайдера, відцентрована по вертикалі
        Rectangle textRightRect = {
            bounds.x + bounds.width + 12,
            bounds.y + bounds.height / 2.0f - totalHeight / 2.0f,
            maxWidth + 2 * padding,
            totalHeight
        };

        DrawRectangleRec(textRightRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(textRightRect, 1, textColor);

        // Виводимо кожен рядок
        for (int i = 0; i < lineCount; i++) {
            DrawTextScaled(font,
                        (int)(textRightRect.x + padding),
                        (int)(textRightRect.y + padding / 2 + i * (lineHeight + LineSpacing)),
                        lines[i], spacing, 1, textColor);
        }
    }

    // --- Відображення числового значення слайдера, лише коли миша наведена ---
    if (mouseOver) {
        // Форматування значення з двома знаками після коми
        char valueText[16];
        snprintf(valueText, sizeof(valueText), "%.2f", *value);

        int paddingVal = 6;
        int charCountVal = utf8_strlen(valueText);
        float valTextWidth = charCountVal * (font.glyph_width + spacing) - spacing;
        float valBoxWidth = valTextWidth + 2 * paddingVal;
        float valBoxHeight = font.glyph_height + 2 * paddingVal;

        // Розташування поля для числового значення праворуч, трохи нижче
        Rectangle valueRect = {
            bounds.x + bounds.width + 12,
            bounds.y + bounds.height / 2.0f + 20,
            valBoxWidth,
            valBoxHeight
        };

        // Малюємо фон і контур поля зі значенням
        DrawRectangleRec(valueRect, baseColor);
        DrawRectangleLinesEx(valueRect, 1, GetContrastingTextColor(baseColor));

        // Виводимо текст значення
        DrawTextScaled(font,
                    valueRect.x + paddingVal,
                    valueRect.y + paddingVal / 2,
                    valueText, spacing, 1, GetContrastingTextColor(baseColor));
    }

    // Повертаємо оновлене значення слайдера
    return *value;
}

