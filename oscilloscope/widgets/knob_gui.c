// knob_gui.c

#include "knob_gui.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define PI 3.14159265358979323846f
#define CHANNEL_COUNT 7

#include "all_font.h"
#include "glyphs.h"

extern int spacing;  // Відступ між символами для малювання тексту

// Зберігаємо кути поворотів регуляторів для кожного каналу
static float knobAngles[CHANNEL_COUNT] = { -135.0f, -135.0f, -135.0f, -135.0f, -135.0f, -135.0f, -135.0f };
// Стан перетягування для кожного каналу
static bool isDragging[CHANNEL_COUNT] = { false, false, false, false, false, false, false };
// Індекс активного каналу для перетягування (один за раз)
static int activeDraggingChannel = -1;

/**
 * @brief Малює скруглений прямокутник із заданими розмірами й кольором.
 *
 * @param x Координата X верхнього лівого кута
 * @param y Координата Y верхнього лівого кута
 * @param width Ширина прямокутника
 * @param height Висота прямокутника
 * @param radius Радіус закруглення кутів
 * @param color Колір для заповнення
 */
static void DrawRoundedRectangle(int x, int y, int width, int height, int radius, Color color)
{
    DrawRectangle(x + radius, y, width - 2 * radius, height, color);
    DrawRectangle(x, y + radius, radius, height - 2 * radius, color);
    DrawRectangle(x + width - radius, y + radius, radius, height - 2 * radius, color);
    DrawCircle(x + radius, y + radius, radius, color);
    DrawCircle(x + width - radius, y + radius, radius, color);
    DrawCircle(x + radius, y + height - radius, radius, color);
    DrawCircle(x + width - radius, y + height - radius, radius, color);
}

/**
 * @brief Намальовує градієнтне кільце навколо центру регулятора для візуального ефекту.
 *
 * @param center Вектор центру кола
 * @param innerRadius Внутрішній радіус кільця
 * @param outerRadius Зовнішній радіус кільця
 * @param innerColor Колір зсередини кільця
 * @param outerColor Колір ззовні кільця
 * @param segments Кількість сегментів кільця
 */
static void DrawGradientRing(Vector2 center, float innerRadius, float outerRadius, Color innerColor, Color outerColor, int segments)
{
    float step = (outerRadius - innerRadius) / segments;

    for (int i = 0; i < segments; i++) {
        float radiusStart = innerRadius + i * step;
        float radiusEnd = radiusStart + step;

        // Лінійна інтерполяція кольору між innerColor і outerColor
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

/**
 * @brief Малює регулятор (knob) із заданим кутом, значенням та кольором.
 *
 * @param font_knob Шрифт для малювання шкали та підписів
 * @param font_value Шрифт для малювання значення всередині регулятора
 * @param x_pos Центр регулятора X
 * @param y_pos Центр регулятора Y
 * @param radius Радіус регулятора
 * @param knobAngle Поточний кут повороту регулятора (градуси)
 * @param knobValue Поточне значення регулятора
 * @param minValue Мінімальне значення регулятора
 * @param maxValue Максимальне значення регулятора
 * @param colorText Колір тексту і індикатора
 */
static void draw_knob(RasterFont font_knob, RasterFont font_value,
                      int x_pos, int y_pos, float radius, float knobAngle, float knobValue,
                      float minValue, float maxValue, Color colorText)
{
    Vector2 center = { (float)x_pos, (float)y_pos };

    // Фон великий круглий світло-сірий
    DrawCircleV(center, radius, LIGHTGRAY);
    // Менший темний круг
    DrawCircleV(center, radius - 5, DARKGRAY);

    // Малюємо градієнтне кільце для ефекту
    DrawGradientRing(center, radius - 12, radius - 5, (Color){200, 200, 200, 255}, (Color){150, 150, 150, 0}, 20);

    // Визначення кольорових зон шкали
    struct Zone {
        float startNorm; // початок норми (0..1)
        float endNorm;   // кінець норми (0..1)
        Color color;     // колір сегмента
    } zones[] = {
        { 0.00f, 0.33f, GREEN },
        { 0.33f, 0.66f, YELLOW },
        { 0.66f, 1.00f, RED }
    };

    float rotationOffset = -90.0f; // Зсув для шкали у градусах

    // Малюємо сегменти шкали з кольорами
    for (int i = 0; i < sizeof(zones)/sizeof(zones[0]); i++) {
        float startAngle = -135.0f + zones[i].startNorm * 270.0f + rotationOffset;
        float endAngle = -135.0f + zones[i].endNorm * 270.0f + rotationOffset;

        DrawRing(center, radius - 12, radius - 5, startAngle, endAngle, 64, Fade(zones[i].color, 0.50f));
    }

    // Малюємо стрілку індикатора положення ручки регулятора
    float indicatorLength = radius;
    float rad = (knobAngle - 90.0f) * (PI / 180.0f); // Конвертуємо в радіани для тригонометрії
    Vector2 indicator = { center.x + cosf(rad) * indicatorLength, center.y + sinf(rad) * indicatorLength };
    DrawLineEx(center, indicator, 4, colorText);

    // Малюємо шкалу рисок і підписів значень кожні 10%
    for (int i = 0; i <= 100; i += 10) {
        float tickAngleDeg = -135.0f + ((float)i / 100.0f) * 270.0f;
        float tickRad = (tickAngleDeg - 90.0f) * (PI / 180.0f);

        float innerRadius = radius + 10;
        float outerRadius = radius;

        Vector2 start = { center.x + cosf(tickRad) * innerRadius, center.y + sinf(tickRad) * innerRadius };
        Vector2 end = { center.x + cosf(tickRad) * outerRadius, center.y + sinf(tickRad) * outerRadius };
        DrawLineEx(start, end, 3, colorText);

        float valueAtTick = minValue + ((float)i / 100.0f) * (maxValue - minValue);
        char buf[16];
        if (maxValue < 10)
            snprintf(buf, sizeof(buf), "%.1f", valueAtTick);
        else
            snprintf(buf, sizeof(buf), "%.0f", valueAtTick);
        int charCount = utf8_strlen(buf);

        float textRadius = innerRadius + (font_knob.glyph_width * charCount) / 2 + 4;
        Vector2 textPos = { center.x + cosf(tickRad) * textRadius, center.y + sinf(tickRad) * textRadius };

        float lineWidth = charCount * (font_knob.glyph_width + spacing) - spacing;
        DrawTextScaled(font_knob, textPos.x - lineWidth/2 + spacing, textPos.y - font_knob.glyph_height/2 + spacing, buf, spacing, 1, colorText);
    }

    // Відображаємо числове значення регулятора під ним у прямокутнику
    char bufValue[32];
    snprintf(bufValue, sizeof(bufValue), "%.1f", knobValue);
    int charCount = utf8_strlen(bufValue);
    float lineWidth = charCount * (font_value.glyph_width + spacing) - spacing;
    float lineHeight = font_value.glyph_height;

    int paddingX = 2; // Відступи по X
    int paddingY = 0; // Відступи по Y

    // Позиція прямокутника зі значенням
    float rectX = x_pos - (lineWidth / 2) - paddingX;
    float rectY = y_pos + radius;
    float rectWidth = lineWidth + 2 * paddingX;
    float rectHeight = lineHeight + 2 * paddingY;

    DrawRectangleRec((Rectangle){rectX, rectY, rectWidth, rectHeight}, colorText);
    DrawRectangleLinesEx((Rectangle){rectX, rectY, rectWidth, rectHeight}, 1, Fade(GetContrastColor(colorText), 0.5f));

    Color textColor = GetContrastColor(colorText);
    DrawTextScaled(font_value, rectX + paddingX, rectY + paddingY, bufValue, spacing, 1, textColor);
}

/**
 * @brief Нормалізує float value у діапазон 0..1 з урахуванням можливої інверсії діапазону.
 *
 * @param value Поточне значення
 * @param minValue Мінімальне значення діапазону
 * @param maxValue Максимальне значення діапазону
 * @return float Нормалізоване значення 0..1
 */
static float NormalizeValue(float value, float minValue, float maxValue)
{
    if (minValue < maxValue) {
        // Прямий діапазон: просте масштабування в межах 0..1
        return (value - minValue) / (maxValue - minValue);
    } else {
        // Інверсний діапазон: інвертуємо щоб кут регулятора обертався правильно
        return 1.0f - (value - maxValue) / (minValue - maxValue);
    }
}

/**
 * @brief Денормалізує float normalized (0..1) назад до значення у діапазоні між minValue та maxValue, враховуючи інверсію.
 *
 * @param normalized Нормалізоване значення 0..1
 * @param minValue Мінімальне значення діапазону
 * @param maxValue Максимальне значення діапазону
 * @return float Розраховане значення
 */
static float DenormalizeValue(float normalized, float minValue, float maxValue)
{
    if (minValue < maxValue) {
        // Прямий діапазон
        return minValue + normalized * (maxValue - minValue);
    } else {
        // Інверсний: 0 дає maxValue, 1 дає minValue
        return maxValue + (1.0f - normalized) * (minValue - maxValue);
    }
}

/**
 * @brief Обробляє взаємодію з регулятором (поворот мишею) та повертає нове значення.
 *
 * @param x_pos X-координата центру регулятора
 * @param y_pos Y-координата центру регулятора
 * @param radius Радіус регулятора
 * @param knobValue Поточне значення регулятора
 * @param isDragging Вказівник на стан перетягування (true/false)
 * @param knobAngle Вказівник на поточний кут повороту
 * @param isActive Чи активний регулятор (реагує на взаємодії)
 * @param channel Індекс каналу (для багатоканального відслідковування)
 * @param minValue Мінімальне значення діапазону
 * @param maxValue Максимальне значення діапазону
 * @return float Оновлене значення регулятора з урахуванням взаємодії
 */
static float knob_handler(int x_pos, int y_pos, float radius, float knobValue, bool* isDragging,
                          float* knobAngle, bool isActive, int channel, float minValue, float maxValue)
{
    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();

    if (!isActive) return knobValue; // Якщо регулятор не активний, повертаємо без змін

    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius); // Чи курсор над регулятором

    // Обчислюємо кут від центру до поточної позиції миші у градусах
    float angle = atan2f(mousePos.y - center.y, mousePos.x - center.x) * (180.0f / PI);
    angle -= 90.0f + 180.0f; // Коригуємо кут для діапазону -135..135 градусів

    // Корекція кута заданим діапазоном
    if (angle < -180.0f) angle += 360.0f;
    if (angle < -135.0f) angle = -135.0f;
    if (angle > 135.0f) angle = 135.0f;

    // Починаємо перетягування кнопкою миші, якщо курсор всередині та ніхто інший не тягне
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver && activeDraggingChannel == -1) {
        *isDragging = true;
        activeDraggingChannel = channel;
    }

    // Завершуємо перетягування при відпусканні кнопки
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *isDragging) {
        *isDragging = false;
        activeDraggingChannel = -1;
    }

    if (*isDragging) {
        *knobAngle = angle;  // Оновлюємо кут

        // Нормалізуємо кут у діапазон 0..1
        float normalizedValue = (*knobAngle + 135.0f) / 270.0f;

        // Переводимо в значення з урахуванням можливого інверсного діапазону
        knobValue = DenormalizeValue(normalizedValue, minValue, maxValue);

        // Обмежуємо значення у межах
        if (minValue < maxValue) {
            if (knobValue < minValue) knobValue = minValue;
            if (knobValue > maxValue) knobValue = maxValue;
        } else {
            if (knobValue > minValue) knobValue = minValue;
            if (knobValue < maxValue) knobValue = maxValue;
        }
    }

    return knobValue;
}

/**
 * @brief Відображає та обробляє поворотний регулятор (knob) для певного каналу.
 *
 * @param channel Номер каналу (індекс від 0 до CHANNEL_COUNT-1)
 * @param font_knob Шрифт для числової шкали регулятора
 * @param font_value Шрифт для відображення значення регулятора
 * @param x_pos Координата X центру регулятора на екрані
 * @param y_pos Координата Y центру регулятора на екрані
 * @param textTop Текст підказки, що відображається зверху при наведенні
 * @param textRight Текст праворуч регулятора
 * @param radius Радіус кола регулятора в пікселях
 * @param value Вказівник на значення регулятора типу float (в межах minValue..maxValue)
 * @param minValue Мінімальне значення регулятора (float)
 * @param maxValue Максимальне значення регулятора (float)
 * @param isActive Чи активний регулятор (чи реагує на події користувача)
 * @param colorText Колір тексту значення регулятора (тип Color з raylib)
 * @return int 1, якщо значення регулятора змінилося, інакше 0.
 */
int Gui_Knob_Channel(int channel, RasterFont font_knob, RasterFont font_value,
                     int x_pos, int y_pos, const char* textTop, const char* textRight,
                     float radius, float* value, float minValue, float maxValue,
                     bool isActive, Color colorText)
{
    if (channel < 0 || channel >= CHANNEL_COUNT) return 0; // Невірний індекс каналу

    // Розміри прямокутника фону регулятора
    int bgWidth = radius / 10 * 37;
    int bgHeight = radius / 10 * 31;
    int bgX = x_pos - bgWidth / 2;
    int bgY = y_pos - bgHeight / 2;
    bgY -= 5; // Трохи піднімаємо фон вгору для балансування

    // Малюємо фон регулятора у вигляді скругленого прямокутника
    DrawRoundedRectangle(bgX, bgY, bgWidth, bgHeight, 4, Fade(GetContrastColor(colorText), 0.8f));

    // Обробляємо взаємодію користувача (перетягування миші) і отримуємо оновлене значення
    float changedValue = knob_handler(x_pos, y_pos, radius, *value, &isDragging[channel], &knobAngles[channel], isActive, channel, minValue, maxValue);

    // Чи відбулося змінення значення
    bool valueChanged = (changedValue != *value);
    *value = changedValue;

    if (!isDragging[channel]) {
        // Якщо не тягнуть, обчислюємо кут на основі поточного значення, ураховуючи нормалізацію
        float normalized = NormalizeValue(*value, minValue, maxValue);
        knobAngles[channel] = normalized * 270.0f - 135.0f;
    }

    // Малюємо регулятор з актуальним кутом та значенням
    draw_knob(font_knob, font_value, x_pos, y_pos, radius, knobAngles[channel], *value, minValue, maxValue, colorText);

    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);

    // Обробка прокручування колеса миші над регулятором
    if (mouseOver) {
        int wheelMove = GetMouseWheelMove();
        if (wheelMove != 0) {
            float step = fabs(maxValue - minValue) / 100.0f; // Крок зміни - 1% від діапазону
            if (minValue < maxValue) {
                *value += wheelMove * step; // Звичайне додавання
                if (*value < minValue) *value = minValue;
                if (*value > maxValue) *value = maxValue;
            } else {
                *value -= wheelMove * step; // Інверсія напрямку для діапазону, де minValue > maxValue
                if (*value > minValue) *value = minValue;
                if (*value < maxValue) *value = maxValue;
            }
            float normalized = NormalizeValue(*value, minValue, maxValue);
            knobAngles[channel] = normalized * 270.0f - 135.0f; // Оновлюємо кут відповідно до нового значення
            valueChanged = true;
        }
    }

    // Визначаємо колір тексту (автоматично підбираємо, якщо альфа 0)
    Color textColor = GetContrastColor(colorText);

    // Відображення підказки (тултіпа) зверху при наведенні курсора миші
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
        DrawRectangleLinesEx(tooltipRect, 1, GetContrastColor(textColor));

        // Малюємо кожен рядок тексту з вертикальним інтервалом 2 пікселі
        for (int i = 0; i < lineCount; i++) {
            DrawTextScaled(font_value,
                           tooltipRect.x + padding * 2, tooltipRect.y + padding / 2 + i * (lineHeight + 2),
                           lines[i], 2, 1, colorText);
        }
    }

    // Відображення тексту праворуч, якщо є
    if (textRight && strlen(textRight) > 0) {
        DrawTextScaled(font_value, x_pos + radius + 10, y_pos - 10, textRight, 1, 1, colorText);
    }

    // Повертаємо 1, якщо значення змінилося, інакше 0
    return valueChanged ? 1 : 0;
}

