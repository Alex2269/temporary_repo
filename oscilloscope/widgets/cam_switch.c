// cam_switch.c - реалізація графічного інтерфейсу rotary cam switch з коментарями

#include "cam_switch.h"  // Заголовочний файл для декларацій, типів і зовнішніх залежностей
#include <math.h>       // Для математичних функцій (atan2f, cosf, sinf)
#include <string.h>     // Для роботи зі строками (strncpy, strlen)
#include <stdio.h>      // Для форматованого виводу рядків (snprintf)

#define PI 3.14159265358979323846f  // Константа числа пі
#define CHANNEL_COUNT 6             // Кількість каналів (перемикачів) у системі

#include "all_font.h"    // Опис шрифтів як структур RasterFont
#include "glyphs.h"      // Гліфи шрифтів

extern int spacing; // Відступ між символами у тексті, узгоджений з RenderFont

// Положення кутів (у градусах) для кожного із CHANNEL_COUNT rotary cam switch
static float camSwitchAngles[CHANNEL_COUNT] = { -135.0f, -135.0f, -135.0f, -135.0f, -135.0f, -135.0f };
// Масив станів перетягування для кожного каналу (true - відбувається перетягування)
static bool isDragging[CHANNEL_COUNT] = { false, false, false, false, false, false };
// Індекс активного каналу при перетягуванні (-1 - жоден не перетягується)
static int activeDraggingChannel = -1;

// Функція малювання скругленого прямокутника, використовується для фону rotary cam switch
static void DrawRoundedRectangle(int x, int y, int width, int height, int radius, Color color) {
    // Малюємо центральну область прямокутника (з урахуванням радіусу округлення)
    DrawRectangle(x + radius, y, width - 2 * radius, height, color);
    // Ліва вертикальна смуга
    DrawRectangle(x, y + radius, radius, height - 2 * radius, color);
    // Права вертикальна смуга
    DrawRectangle(x + width - radius, y + radius, radius, height - 2 * radius, color);
    // Чотири кути - кола для округлення
    DrawCircle(x + radius, y + radius, radius, color);
    DrawCircle(x + width - radius, y + radius, radius, color);
    DrawCircle(x + radius, y + height - radius, radius, color);
    DrawCircle(x + width - radius, y + height - radius, radius, color);
}

// Функція малювання градієнтного обідка навколо rotary cam switch, для кращого ефекту
static void DrawGradientRing(Vector2 center, float innerRadius, float outerRadius,
                             Color innerColor, Color outerColor, int segments)
{
    float step = (outerRadius - innerRadius) / segments; // Крок радіусу для кожного сегменту

    // Ітеруємося по сегментах кільця
    for (int i = 0; i < segments; i++) {
        float radiusStart = innerRadius + i * step;  // Внутрішній радіус сегмента
        float radiusEnd = radiusStart + step;        // Зовнішній радіус сегмента

        // Лінійна інтерполяція кольору між innerColor та outerColor для плавного переходу
        float t = (float)i / (segments - 1);
        Color color = {
            (unsigned char)((1 - t) * innerColor.r + t * outerColor.r),
            (unsigned char)((1 - t) * innerColor.g + t * outerColor.g),
            (unsigned char)((1 - t) * innerColor.b + t * outerColor.b),
            (unsigned char)((1 - t) * innerColor.a + t * outerColor.a)
        };

        // Малюємо сегмент кільця з інтерполірованим кольором
        DrawRing(center, radiusStart, radiusEnd, 0, 360, 64, color);
    }
}

// Функція малювання rotary cam switch (ручки), із кольоровою шкалою та значеннями
static void draw_camSwitch(RasterFont font_knob, RasterFont font_value,
                           int x_pos, int y_pos, float radius, float angle, float value,
                           float minValue, float maxValue, Color colorText)
{
    Vector2 center = { (float)x_pos, (float)y_pos };

    // Малюємо дві концентричні окружності для базового вигляду ручки
    DrawCircleV(center, radius, LIGHTGRAY);
    DrawCircleV(center, radius - 5, DARKGRAY);

    // Малюємо градієнтний обідок для декоративного ефекту
    DrawGradientRing(center, radius - 12, radius - 5,
                     (Color){200, 200, 200, 255}, (Color){150, 150, 150, 0}, 20);

    // Визначаємо кольорові зони шкали: зелена (0-33%), жовта (33-66%), червона (66-100%)
    struct Zone {
        float startNorm; // початкове нормалізоване значення (0..1)
        float endNorm;   // кінцеве нормалізоване значення (0..1)
        Color color;     // колір зони
    } zones[] = {
        { 0.00f, 0.33f, GREEN },
        { 0.33f, 0.66f, YELLOW },
        { 0.66f, 1.00f, RED }
    };

    float rotationOffset = -90.0f; // Зсув кута для налаштування розташування шкали

    // Малюємо кольорові сегменти шкали за допомогою кола з прозорістю 50%
    for (int i = 0; i < sizeof(zones)/sizeof(zones[0]); i++) {
        float startAngle = -135.0f + zones[i].startNorm * 270.0f + rotationOffset;
        float endAngle = -135.0f + zones[i].endNorm * 270.0f + rotationOffset;
        DrawRing(center, radius - 12, radius - 5, startAngle, endAngle, 64, Fade(zones[i].color, 0.50f));
    }

    // Малюємо лінію-індикатор поточного положення rotary cam switch
    float indicatorLength = radius;
    float rad = (angle - 90.0f) * (PI / 180.0f); // Перетворення градусів в радіани, з поправкою
    Vector2 indicator = { center.x + cosf(rad) * indicatorLength, center.y + sinf(rad) * indicatorLength };
    DrawLineEx(center, indicator, 4, colorText); // Лінія товщиною 4 пікселі потрібного кольору

    // Малюємо шкалу рисок і числові мітки через кожних 10%
    for (int i = 0; i <= 100; i += 10) {
        // Кут для кожної риски (0 - 100% шкали)
        float tickAngleDeg = -135.0f + (i / 100.0f) * 270.0f;
        float tickRad = (tickAngleDeg - 90.0f) * (PI / 180.0f);

        float innerRadius = radius + 10; // Довжина риски в напрямку назовні від ручки
        float outerRadius = radius;      // Початкова точка риски (край ручки)

        // Обчислення початкової та кінцевої точки риски
        Vector2 start = { center.x + cosf(tickRad) * innerRadius, center.y + sinf(tickRad) * innerRadius };
        Vector2 end = { center.x + cosf(tickRad) * outerRadius, center.y + sinf(tickRad) * outerRadius };
        DrawLineEx(start, end, 3, colorText); // Малюємо риску товщиною 3 пікселі

        // Обчислення значення мітки відповідно до позиції на шкалі
        float valueAtTick = minValue + (i / 100.0f) * (maxValue - minValue);
        char buf[16];
        if (maxValue < 10)
            snprintf(buf, sizeof(buf), "%.1f", valueAtTick);
        else
            snprintf(buf, sizeof(buf), "%.0f", valueAtTick);

        int charCount = utf8_strlen(buf);

        // Визначаємо позицію тексту для мітки, на відстані від ручки
        float textRadius = innerRadius + (font_knob.glyph_width * charCount) / 2 + 4;
        Vector2 textPos = { center.x + cosf(tickRad) * textRadius, center.y + sinf(tickRad) * textRadius };

        float lineWidth = charCount * (font_knob.glyph_width + spacing) - spacing;
        // Малюємо числову мітку текстом по центру відносно текстової позиції
        DrawTextScaled(font_knob, textPos.x - lineWidth / 2 + spacing, textPos.y - font_knob.glyph_height / 2 + spacing, buf, spacing, 1, colorText);
    }

    // Відображення поточного числового значення rotary cam switch під ручкою
    char bufValue[32];
    snprintf(bufValue, sizeof(bufValue), "%.1f", value);
    int charCount = utf8_strlen(bufValue);
    float lineWidth = charCount * (font_value.glyph_width + spacing) - spacing;
    float lineHeight = font_value.glyph_height;

    // Відступи всередині прямокутника для поточного значення
    int paddingX = 2;
    int paddingY = 0;

    // Позиція та розмір прямокутника під значенням, по центру під ручкою
    float rectX = x_pos - (lineWidth / 2) - paddingX;
    float rectY = y_pos + radius; // Трохи під ручкою
    float rectWidth = lineWidth + 2 * paddingX;
    float rectHeight = lineHeight + 2 * paddingY;

    // Малюємо фон прямокутника з поточним значенням кольором rotary cam switch
    DrawRectangleRec((Rectangle){rectX, rectY, rectWidth, rectHeight}, colorText);
    // Малюємо окантовку прямокутника з напівпрозорим контрастним кольором
    DrawRectangleLinesEx((Rectangle){rectX, rectY, rectWidth, rectHeight}, 1, Fade(GetContrastColor(colorText), 0.5f));

    // Визначаємо колір тексту (чорний чи білий) залежно від кольору фону
    Color textColor = GetContrastColor(colorText);
    // Малюємо текстове значення поточного положення rotary cam switch
    DrawTextScaled(font_value, rectX + paddingX, rectY + paddingY, bufValue, spacing, 1, textColor);
}

// Нормалізує значення у діапазон 0..1, враховуючи інверсію
static float NormalizeValue(float value, float minValue, float maxValue)
{
    if (minValue < maxValue) {
        return (value - minValue) / (maxValue - minValue);
    } else {
        return 1.0f - (value - maxValue) / (minValue - maxValue);
    }
}

// Денормалізує значення з 0..1 в діапазон з урахуванням інверсії
static float DenormalizeValue(float normalized, float minValue, float maxValue)
{
    if (minValue < maxValue) {
        return minValue + normalized * (maxValue - minValue);
    } else {
        return maxValue + (1.0f - normalized) * (minValue - maxValue);
    }
}

// Обробка взаємодії користувача з rotary cam switch (поворотом ручки)
// Параметри:
// - x_pos, y_pos - центр rotary cam switch
// - radius - радіус сфери взаємодії
// - value - поточне значення rotary cam switch
// - dragging - вказівник на стан перетягування (true/false)
// - angle - поточний кут повороту ручки за градусами
// - isActive - активність взаємодії (натиснуто)
// - channel - номер каналу rotary cam switch
// - minValue, maxValue - діапазон значень rotary cam switch
static float camSwitch_handler(int x_pos, int y_pos, float radius, float value, bool *dragging,
                               float *angle, bool isActive, int channel, float minValue, float maxValue)
{
    Vector2 center = { (float)x_pos, (float)y_pos };     // Центр rotary cam switch
    Vector2 mousePos = GetMousePosition();               // Поточна позиція курсора миші

    if (!isActive) return value;  // Якщо rotary cam switch неактивний, повертаємо поточне значення

    // Перевіряємо чи курсор знаходиться в межах радиуса rotary cam switch
    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);

    // Обчислення кута між центром rotary cam switch і курсором миші
    float ang = atan2f(mousePos.y - center.y, mousePos.x - center.x) * (180.0f / PI);
    ang -= 90.0f + 180.0f; // Зсув кута для коректного орієнтування

    // Коригуємо кут для обмеження в межах -135 ... +135 градусів
    if (ang < -180.0f) ang += 360.0f;
    if (ang < -135.0f) ang = -135.0f;
    if (ang > 135.0f) ang = 135.0f;

    // Обробка початку перетягування - натиснення ЛКМ на rotary cam switch та відсутність інших перетягувань
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver && activeDraggingChannel == -1) {
        *dragging = true;
        activeDraggingChannel = channel;
    }

    // При відпусканні ЛКМ завершуємо перетягування
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *dragging) {
        *dragging = false;
        activeDraggingChannel = -1;
    }

    // Якщо триває перетягування - оновлюємо кут і значення rotary cam switch
    if (*dragging) {
        *angle = ang;

        // Нормалізуємо кут в 0..1
        float normalized = (*angle + 135.0f) / 270.0f;

        // Денормалізуємо з урахуванням інверсії
        value = DenormalizeValue(normalized, minValue, maxValue);

        // Обмеження в межах
        if (minValue < maxValue) {
            if (value < minValue) value = minValue;
            if (value > maxValue) value = maxValue;
        } else {
            if (value > minValue) value = minValue;
            if (value < maxValue) value = maxValue;
        }
    }

    return value; // Повертаємо оновлене або попереднє значення rotary cam switch
}

// Графічний інтерфейс rotary cam switch для певного каналу
// Параметри:
// - channel - номер каналу (0..CHANNEL_COUNT-1)
// - font_knob, font_value - шрифти для рисок та значення
// - x_pos, y_pos - позиція центру rotary cam switch
// - tooltipTop - підказка при наведенні курсора
// - textRight - текст зправа від rotary cam switch
// - radius - радіус rotary cam switch
// - value - вказівник на поточне значення rotary cam switch
// - minValue, maxValue - діапазон значень
// - isActive - активність rotary cam switch (чи реагує на взаємодію)
// - colorText - колір тексту і елементів перемикача
int Gui_CamSwitch_Channel(int channel, RasterFont font_knob, RasterFont font_value,
                          int x_pos, int y_pos, const char *tooltipTop, const char *textRight,
                          float radius, float *value, float minValue, float maxValue, bool isActive, Color colorText)
{
    // Перевірка коректності номера каналу
    if (channel < 0 || channel >= CHANNEL_COUNT) return 0;

    // Визначаємо розміри фону rotary cam switch як пропорції від радіуса
    int bgWidth = radius / 10 * 37;
    int bgHeight = radius / 10 * 31;
    int bgX = x_pos - bgWidth / 2; // Ліва верхня координата по Х
    int bgY = y_pos - bgHeight / 2; // Ліва верхня координата по Y
    bgY -= 5; // Невеликий зсув фону вгору для кращого розташування

    // Малюємо фон rotary cam switch (скруглений прямокутник)
    DrawRoundedRectangle(bgX, bgY, bgWidth, bgHeight, 4, Fade(GetContrastColor(colorText), 0.8f));

    // Обробляємо взаємодію користувача з rotary cam switch і отримуємо оновлене значення
    float changed = camSwitch_handler(x_pos, y_pos, radius, *value, &isDragging[channel], &camSwitchAngles[channel], isActive, channel, minValue, maxValue);
    bool valueChanged = fabsf(changed - *value) > 0.001f; // Порівнюємо зміни з урахуванням точності
    *value = changed; // Оновлюємо зовнішнє значення

    // Кількість рисок на шкалі і кроки - фіксовані
    const int tickCount = 11;
    float valueStep = (maxValue - minValue) / (tickCount - 1); // Крок між значеннями на рисках
    float angleStep = 270.0f / (tickCount - 1);               // Кутовий крок між рисками в градусах

    // Якщо rotary cam switch не перетягується - округлюємо значення до найближчої риски
    if (!isDragging[channel]) {
        float normalized = NormalizeValue(*value, minValue, maxValue);

        float roundedNormalized = roundf(normalized * (tickCount - 1)) / (tickCount - 1);

        *value = DenormalizeValue(roundedNormalized, minValue, maxValue);

        camSwitchAngles[channel] = roundedNormalized * 270.0f - 135.0f;
    }

    // Малюємо rotary cam switch з оновленими параметрами кута та значення
    draw_camSwitch(font_knob, font_value, x_pos, y_pos, radius, camSwitchAngles[channel], *value, minValue, maxValue, colorText);

    // Позиція центра rotary cam switch та поточна позиція курсора миші
    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();

    // Перевірка, чи курсор знаходиться на rotary cam switch
    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);
    // Перевіряємо, чи курсор миші над rotary cam switch (в межах радіуса)

    if (mouseOver) {
        int wheelMove = GetMouseWheelMove();
        if (wheelMove != 0) {
            float step = fabs(maxValue - minValue) / (tickCount - 1);
            if (minValue < maxValue) {
                *value += wheelMove * step;
                if (*value < minValue) *value = minValue;
                if (*value > maxValue) *value = maxValue;
            } else {
                *value -= wheelMove * step;
                if (*value > minValue) *value = minValue;
                if (*value < maxValue) *value = maxValue;
            }
            float normalized = NormalizeValue(*value, minValue, maxValue);
            camSwitchAngles[channel] = normalized * 270.0f - 135.0f;
            valueChanged = true;
        }
    }

    Color textColor = GetContrastColor(colorText);
    // Визначаємо контрастний до фону колір тексту (білий або чорний)

    // Малюємо риски шкали та числові мітки для позначення положення rotary cam switch
    for (int i = 0; i < tickCount; i++) {
        float tickValue = minValue + i * valueStep;          // Значення на рисці
        float tickAngleDeg = -135.0f + i * angleStep;        // Кут в градусах для риски
        float tickRad = (tickAngleDeg - 90.0f) * (PI / 180.0f); // Кут у радіанах для малювання

        float innerRadius = radius + 10;  // Відстань від центра до початку риски
        float outerRadius = radius;       // Відстань до кінця риски (край диску)

        // Обчислення координат початку риски
        Vector2 start = { center.x + cosf(tickRad) * innerRadius, center.y + sinf(tickRad) * innerRadius };
        // Обчислення координат кінця риски
        Vector2 end = { center.x + cosf(tickRad) * outerRadius, center.y + sinf(tickRad) * outerRadius };
        DrawLineEx(start, end, 3, colorText);  // Малюємо риску товщиною 3 пікселі

        // Форматування тексту мітки
        char buf[16];
        if (maxValue < 10)
            snprintf(buf, sizeof(buf), "%.1f", tickValue);   // Якщо max < 10 – 1 знак після коми
            else
                snprintf(buf, sizeof(buf), "%.0f", tickValue);   // Інакше без десяткових

                int charCount = utf8_strlen(buf);

            // Визначаємо позицію текстової мітки на деякій відстані від риски
            float textRadius = innerRadius + (font_knob.glyph_width * charCount) / 2 + 4;
        Vector2 textPos = { center.x + cosf(tickRad) * textRadius, center.y + sinf(tickRad) * textRadius };

        // Обчислення ширини тексту для центрованого розташування
        float lineWidth = charCount * (font_knob.glyph_width + spacing) - spacing;
        // Малюємо текстову мітку по центру у відповідній позиції
        DrawTextScaled(font_knob, textPos.x - lineWidth / 2 + spacing, textPos.y - font_knob.glyph_height / 2 + spacing, buf, spacing, 1, colorText);
    }

    // Якщо курсор над rotary cam switch і є підказка, малюємо її зверху
    if (mouseOver && tooltipTop && strlen(tooltipTop) > 0) {
        const int padding = 6;  // Відступи навколо тексту підказки

        const char* lines[10];  // Масив рядків для багато рядкової підказки
        int lineCount = 0;

        // Копіюємо текст у тимчасовий буфер, щоб безпечно розділити на рядки
        char tempText[256];
        strncpy(tempText, tooltipTop, sizeof(tempText) - 1);
        tempText[sizeof(tempText) - 1] = '\0';

        // Розбиваємо текст на рядки за символом '\n'
        char* line = strtok(tempText, "\n");
        while (line != NULL && lineCount < 10) {
            lines[lineCount++] = line;
            line = strtok(NULL, "\n");
        }

        // Обчислюємо максимальну ширину всіх рядків
        float maxWidth = 0;
        for (int i = 0; i < lineCount; i++) {
            int charCount = utf8_strlen(lines[i]);
            float lineWidth = charCount * (font_value.glyph_width + spacing) - spacing + padding;
            if (lineWidth > maxWidth) maxWidth = lineWidth;
        }

        float lineHeight = (float)font_value.glyph_height;  // Висота рядка тексту
        float totalHeight = lineCount * lineHeight + (lineCount - 1) * 2 + padding;  // Загальна висота підказки з інтервалом

        // Прямокутник для області під підказку (центруємо по Х)
        Rectangle tooltipRect = {
            center.x - (maxWidth / 2.0f) - padding,
            center.y - radius - totalHeight - padding / 2,
            maxWidth + 2 * padding,
            totalHeight + padding / 2
        };

        // Малюємо напівпрозорий прямокутник фону підказки
        DrawRectangleRec(tooltipRect, Fade(textColor, 0.8f));
        // Окантовка прямокутника підказки
        DrawRectangleLinesEx(tooltipRect, 1, GetContrastColor(textColor));

        // Малюємо рядки тексту підказки з вертикальним інтервалом 2 пікселі
        for (int i = 0; i < lineCount; i++) {
            DrawTextScaled(font_value, tooltipRect.x + padding * 2, tooltipRect.y + padding / 2 + i * (lineHeight + 2), lines[i], 2, 1, colorText);
        }
    }

    // Якщо є текст праворуч від rotary cam switch – малюємо його
    if (textRight && strlen(textRight) > 0) {
        DrawTextScaled(font_value, x_pos + radius + 10, y_pos - 10, textRight, 1, 1, colorText);
    }

    // Повертаємо 1 якщо значення змінилося, інакше 0
    return valueChanged ? 1 : 0;
}

