// DrawVerticalScale.c

#include "DrawVerticalScale.h"
#include <stdio.h>

// Функція малює вертикальну шкалу з мітками у вказаній області.
// Параметри:
//  channel - номер каналу (поки не використовується, можливо, для майбутніх розширень)
//  scale - масштаб у пікселях на одиницю значення (визначає щільність відображення шкали)
//  offset_y - зміщення по осі Y, центральне значення шкали
//  area - прямокутна область (x, y, width, height), де малюється шкала
//  font - шрифт для виведення числових міток
//  color - колір ліній і тексту шкали
void DrawVerticalScale(int channel, float scale, float offset_y, Rectangle area, RasterFont font, Color color)
{
    int spacing = 2;  // Відступ тексту від рисок шкали в пікселях
    int x_end = area.x + area.width; // X координата правої межі області, де малюємо шкалу
    float centerY = area.y + area.height / 2.0f; // Y координата центру області по вертикалі
    float margin = font.glyph_height; // Відступ зверху і знизу для обмеження малювання міток і тексту
    float y_min = area.y + margin;              // Верхня межа для малювання міток
    float y_max = area.y + area.height - margin; // Нижня межа для малювання міток

    // Обчислюємо мінімальне і максимальне значення шкали відповідно до обмеженої висоти,
    // враховуючи масштаб (пікселі на одиницю) і центральне зміщення offset_y
    float minVal = offset_y - ((area.height / 2.0f - margin) / scale);
    float maxVal = offset_y + ((area.height / 2.0f - margin) / scale);

    // Мінімальна відстань між мітками в пікселях, щоб текст не накладався; 1.5 висоти шрифту
    float min_label_spacing_px = font.glyph_height * 2.5f;

    // Діапазон значень, які треба відобразити на шкалі
    float value_range = maxVal - minVal;

    // Оцінка максимальної кількості міток для області з таким відступом
    float labels_count = area.height / min_label_spacing_px;

    // Сирий крок міток у значеннях шкали
    // float step_raw = value_range / labels_count;
    // float step;
    //
    // // Округлення кроку до "зручних" стандартних значень (25, 50, 75, 100)
    // if (step_raw <= 25.0f) step = 25.0f;
    // else if (step_raw <= 50.0f) step = 50.0f;
    // else if (step_raw <= 75.0f) step = 75.0f;
    // else if (step_raw <= 100.0f) step = 100.0f;
    // else if (step_raw <= 150.0f) step = 150.0f;
    // else if (step_raw <= 200.0f) step = 200.0f;
    // else if (step_raw <= 250.0f) step = 250.0f;
    // else step = 250.0f;

    // Малюємо мітки на шкалі кожні 50 пікселів по вертикалі незалежно від масштабу.
    float step = 50.0f / scale;

    // Малюємо основну вертикальну лінію шкали праворуч в межах області
    DrawLine(x_end, area.y, x_end, area.y + area.height, color);

    // Перебираємо значення від найближчого кратного кроку до мінімального значення до максимального
    for (float val = ((int)(minVal / step)) * step; val <= maxVal; val += step)
    {
        // Обчислюємо Y позицію для мітки, масштабуючи відносно offset_y
        // float y = centerY + (val - offset_y) * scale;

        // Інвертований напрямок шкали по осі Y (числа зростають вниз)
        float y = centerY - (val - offset_y) * scale;

        // Пропускаємо мітки, які знаходяться поза межами обмеженого інтервалу по вертикалі
        if (y < y_min || y > y_max) continue;

        // Формуємо рядок тексту для мітки (цілі значення)
        char label[16];
        snprintf(label, sizeof(label), "%.0f", val);

        // Обчислюємо ширину тексту у пікселях, наприклад, як кількість символів * ширина символу
        // Якщо ваш PSF_Font дає точну метрику ширини тексту, краще використайте її
        int text_width = utf8_strlen(label) * font.glyph_width;

        // Малюємо риску справа від основної вертикальної лінії
        DrawLine(x_end, y, x_end + 10, y, color);

        // Малюємо текст праворуч від риски, відцентрований по горизонталі відносно риски
        DrawTextScaled(font,
                       x_end + 25 + spacing - text_width / 2, y - font.glyph_height / 2,
                       label, spacing, 1, color);
    }
}

