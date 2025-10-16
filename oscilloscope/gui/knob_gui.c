#include "knob_gui.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846f
#define CHANNEL_COUNT 6

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "glyphs.h"

extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

static float knobAngles[CHANNEL_COUNT] = { -135.0f, -135.0f, -135.0f, -135.0f, -135.0f, -135.0f };
static bool isDragging[CHANNEL_COUNT] = { false, false, false, false, false, false };
static int activeDraggingChannel = -1;

// Обчислення яскравості кольору (luminance)
static float GetLuminance(Color color);
// Вибір контрастного кольору тексту (білий або чорний)
static Color GetContrastingTextColor(Color bgColor);

// Функція для малювання скругленого прямокутника
static void DrawRoundedRectangle(int x, int y, int width, int height, int radius, Color color) {
    DrawRectangle(x + radius, y, width - 2 * radius, height, color);
    DrawRectangle(x, y + radius, radius, height - 2 * radius, color);
    DrawRectangle(x + width - radius, y + radius, radius, height - 2 * radius, color);
    DrawCircle(x + radius, y + radius, radius, color);
    DrawCircle(x + width - radius, y + radius, radius, color);
    DrawCircle(x + radius, y + height - radius, radius, color);
    DrawCircle(x + width - radius, y + height - radius, radius, color);
}

// Малюємо градієнтний обідок навколо регулятора
static void DrawGradientRing(Vector2 center, float innerRadius, float outerRadius, Color innerColor, Color outerColor, int segments) {
    float step = (outerRadius - innerRadius) / segments;

    for (int i = 0; i < segments; i++) {
        float radiusStart = innerRadius + i * step;
        float radiusEnd = radiusStart + step;

        // Інтерполяція кольору між innerColor і outerColor
        float t = (float)i / (segments - 1);
        Color color = {
            (unsigned char)((1 - t) * innerColor.r + t * outerColor.r),
            (unsigned char)((1 - t) * innerColor.g + t * outerColor.g),
            (unsigned char)((1 - t) * innerColor.b + t * outerColor.b),
            (unsigned char)((1 - t) * innerColor.a + t * outerColor.a)
        };

        DrawRing(center, radiusStart, radiusEnd, 0, 360, 64, color);
    }
}

// Малюємо сам регулятор (knob)
static void draw_knob(RasterFont font_knob, RasterFont font_value,
                      int x_pos, int y_pos, float radius, float knobAngle, float knobValue,
                      float minValue, float maxValue, Color colorText) {
    Vector2 center = { (float)x_pos, (float)y_pos };

    // Малюємо фон регулятора
    DrawCircleV(center, radius, LIGHTGRAY);
    DrawCircleV(center, radius - 5, DARKGRAY);

    // Малюємо світлосірий градієнтний обідок
    DrawGradientRing(center, radius - 12, radius - 5, (Color){200, 200, 200, 255}, (Color){150, 150, 150, 0}, 20);

    // --- Малюємо кольорові зони шкали ---
    struct Zone {
        float startNorm;
        float endNorm;
        Color color;
    } zones[] = {
        { 0.00f, 0.33f, GREEN },
        { 0.33f, 0.66f, YELLOW },
        { 0.66f, 1.00f, RED }
    };

    float rotationOffset = -90.0f;

    for (int i = 0; i < sizeof(zones)/sizeof(zones[0]); i++) {
        float startAngle = -135.0f + zones[i].startNorm * 270.0f + rotationOffset;
        float endAngle = -135.0f + zones[i].endNorm * 270.0f + rotationOffset;

        DrawRing(center,
                 radius - 12,
                 radius - 5,
                 startAngle,
                 endAngle,
                 64,
                 Fade(zones[i].color, 0.50f));
    }

    // Малюємо індикаторну лінію
    float indicatorLength = radius;
    float rad = (knobAngle - 90.0f) * (PI / 180.0f);
    Vector2 indicator = { center.x + cosf(rad) * indicatorLength, center.y + sinf(rad) * indicatorLength };
    DrawLineEx(center, indicator, 4, colorText);

    // Малюємо шкалу рисок і підписи значень (кожні 10%)
    for (int i = 0; i <= 100; i += 10) {
        float tickAngleDeg = -135.0f + (i / 100.0f) * 270.0f;
        float tickRad = (tickAngleDeg - 90.0f) * (PI / 180.0f);

        float innerRadius = radius + 10;
        float outerRadius = radius;

        Vector2 start = { center.x + cosf(tickRad) * innerRadius, center.y + sinf(tickRad) * innerRadius };
        Vector2 end = { center.x + cosf(tickRad) * outerRadius, center.y + sinf(tickRad) * outerRadius };
        DrawLineEx(start, end, 3, colorText);

        float valueAtTick = minValue + (i / 100.0f) * (maxValue - minValue);
        char buf[16];
        if (maxValue < 10)
            snprintf(buf, sizeof(buf), "%.1f", valueAtTick);
        else
            snprintf(buf, sizeof(buf), "%.0f", valueAtTick);
        int charCount = utf8_strlen(buf);

        float textRadius = innerRadius + (font_knob.glyph_width * charCount) / 2 + 4;
        Vector2 textPos = { center.x + cosf(tickRad) * textRadius, center.y + sinf(tickRad) * textRadius };

        float lineWidth = charCount * (font_knob.glyph_width + spacing) - spacing;
        DrawTextScaled(font_knob,
                       textPos.x - lineWidth/2 + spacing, textPos.y - font_knob.glyph_height/2 + spacing, buf,
                       spacing, 1, colorText);
    }

    // Відображення поточного значення регулятора в прямокутному полі
    char bufValue[32];
    snprintf(bufValue, sizeof(bufValue), "%.1f", knobValue);
    int charCount = utf8_strlen(bufValue);
    float lineWidth = charCount * (font_value.glyph_width + spacing) - spacing;
    float lineHeight = font_value.glyph_height;

    // Відступи всередині прямокутника
    int paddingX = 2/*6*/;
    int paddingY = 0/*4*/;

    // Координати прямокутника (відцентровані по x)
    float rectX = x_pos - (lineWidth / 2) - paddingX;
    float rectY = y_pos + radius;
    float rectWidth = lineWidth + 2 * paddingX;
    float rectHeight = lineHeight + 2 * paddingY;

    DrawRectangleRec((Rectangle){rectX, rectY, rectWidth, rectHeight}, colorText);
    // Малюємо рамку навколо прямокутника
    DrawRectangleLinesEx((Rectangle){rectX, rectY, rectWidth, rectHeight}, 1,
                         Fade(GetContrastingTextColor(colorText), 0.5f));

    // Визначаємо контрастний колір тексту
    Color textColor = GetContrastingTextColor(colorText);

    // Малюємо текст по центру прямокутника з урахуванням padding
    DrawTextScaled(font_value, rectX + paddingX, rectY + paddingY, bufValue, spacing, 1, textColor);
}

// Обробка взаємодії з регулятором
static float knob_handler(int x_pos, int y_pos, float radius, float knobValue, bool *isDragging,
                          float *knobAngle, bool isActive, int channel, float minValue, float maxValue) {
    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();

    if (!isActive) return knobValue;

    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);

    float angle = atan2f(mousePos.y - center.y, mousePos.x - center.x) * (180.0f / PI);
    angle -= 90.0f + 180.0f;

    if (angle < -180.0f) angle += 360.0f;
    if (angle < -135.0f) angle = -135.0f;
    if (angle > 135.0f) angle = 135.0f;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver && activeDraggingChannel == -1) {
        *isDragging = true;
        activeDraggingChannel = channel;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *isDragging) {
        *isDragging = false;
        activeDraggingChannel = -1;
    }

    if (*isDragging) {
        *knobAngle = angle;

        float normalizedValue = (*knobAngle + 135.0f) / 270.0f;
        knobValue = minValue + normalizedValue * (maxValue - minValue);

        if (knobValue < minValue) knobValue = minValue;
        if (knobValue > maxValue) knobValue = maxValue;
    }

    return knobValue;
}

int Gui_Knob_Channel(int channel, RasterFont font_knob, RasterFont font_value,
                     int x_pos, int y_pos, const char *textTop, const char *textRight,
                     float radius, float *value, float minValue, float maxValue, bool isActive, Color colorText) {
    if (channel < 0 || channel >= CHANNEL_COUNT) return 0;

    // Малюємо фон зі скругленим прямокутником
    int bgWidth = radius / 10 * 37;
    int bgHeight = radius / 10 * 31;
    int bgX = x_pos - bgWidth / 2;
    int bgY = y_pos - bgHeight / 2;
    bgY -= 5;  // Зсув фону вгору на 5 пікселів

    DrawRoundedRectangle(bgX, bgY, bgWidth, bgHeight, 4,
                         Fade(GetContrastingTextColor(colorText), 0.8f));

    float changed = knob_handler(x_pos, y_pos, radius, *value, &isDragging[channel], &knobAngles[channel], isActive, channel, minValue, maxValue);
    bool valueChanged = (changed != *value);
    *value = changed;

    if (!isDragging[channel]) {
        knobAngles[channel] = ((*value - minValue) / (maxValue - minValue)) * 270.0f - 135.0f;
    }

    draw_knob(font_knob, font_value, x_pos, y_pos, radius, knobAngles[channel], *value, minValue, maxValue, colorText);

    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);

    // Визначаємо колір тексту (автоматично підбираємо, якщо альфа 0)
    Color textColor = GetContrastingTextColor(colorText);

    // Відображення підказки зверху при наведенні курсора миші
    if (mouseOver && textTop && strlen(textTop) > 0) {
        int padding = 6; // Відступи навколо тексту підказки

        // Розбиваємо текст на рядки за символом '\n'
        // Для простоти використовуємо тимчасовий буфер (припустимо, max 10 рядків)
        const char* lines[10];
        int lineCount = 0;

        // Тимчасовий буфер для розбиття (копія тексту)
        char tempText[256];
        strncpy(tempText, textTop, sizeof(tempText) - 1);
        tempText[sizeof(tempText) - 1] = '\0';

        char* line = strtok(tempText, "\n");
        while (line != NULL && lineCount < 10) {
            lines[lineCount++] = line;
            line = strtok(NULL, "\n");
        }

        // Обчислюємо максимальну ширину серед рядків
        float maxWidth = 0;
        for (int i = 0; i < lineCount; i++) {
            // float lineWidth = (float)font_value.glyph_width * strlen(lines[i]) + (strlen(lines[i]) - 1) * spacing + padding;

            int charCount = utf8_strlen(lines[i]);
            float lineWidth = charCount * (font_value.glyph_width + spacing) - spacing + padding;
            if (lineWidth > maxWidth) maxWidth = lineWidth;
        }

        // Висота одного рядка
        float lineHeight = (float)font_value.glyph_height;

        // Загальна висота прямокутника з урахуванням усіх рядків і відступів
        float totalHeight = lineCount * lineHeight + (lineCount - 1) * 2 + padding;

        // Прямокутник для фону підказки textTop
        Rectangle tooltipRect = {
            center.x - (maxWidth / 2.0f) - padding,
            center.y - radius - totalHeight - padding / 2,
            maxWidth + 2 * padding,
            totalHeight + padding / 2
        };

        // Малюємо фоновий прямокутник підказки textTop з напівпрозорим кольором
        DrawRectangleRec(tooltipRect, Fade(textColor, 0.8f));
        // Малюємо рамку підказки
        DrawRectangleLinesEx(tooltipRect, 1, GetContrastingTextColor(textColor));

        // Малюємо кожен рядок тексту з вертикальним інтервалом 2 пікселі
        for (int i = 0; i < lineCount; i++) {
            DrawTextScaled(font_value,
                           tooltipRect.x + padding * 2, tooltipRect.y + padding / 2 + i * (lineHeight + 2),
                           lines[i], 2, 1, colorText);
        }
    }

    // Відображення тексту зверху (підказка)
    // if (textTop && strlen(textTop) > 0) {
    //     int textWidth = MeasureText(textTop, 14);
    //     DrawTextScaled(font_value, x_pos - textWidth / 2, bgY - 50, textTop, 1, 1, colorText);
    // }

    // Відображення тексту справа
    if (textRight && strlen(textRight) > 0) {
        DrawTextScaled(font_value, x_pos + radius + 10, y_pos - 10, textRight, 1, 1, colorText);
    }

    return valueChanged ? 1 : 0;
}


// Обчислення яскравості кольору (luminance)
static float GetLuminance(Color color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Вибір контрастного кольору тексту (білий або чорний)
static Color GetContrastingTextColor(Color bgColor)
{
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

