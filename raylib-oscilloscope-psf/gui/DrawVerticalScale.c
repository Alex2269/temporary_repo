// DrawVerticalScale.c

#include "DrawVerticalScale.h"

// Функція малює вертикальну шкалу з мітками у вказаній області.
// Параметри:
//  channel - номер каналу (поки не використовується, можливо, для майбутніх розширень)
//  scale - масштаб у пікселях на одиницю значення (як щільно відображати шкалу)
//  offset_y - зміщення по осі Y, центральне значення шкали
//  area - прямокутна область (x, y, width, height), де малюється шкала
//  font - шрифт для виведення числових міток
//  color - колір ліній і тексту шкали
void DrawVerticalScale(int channel, float scale, float offset_y, Rectangle area, PSF_Font font, Color color)
{
    int spacing = 2;  // Відступ для тексту від рисок шкали (прохання в пікселях)
    int x_end = area.x + area.width; // X координата правої межі області, де малюємо шкалу
    float centerY = area.y + area.height / 2.0f; // Y координата центра області по вертикалі

    // Обчислюємо мінімальне і максимальне значення шкали, яке вміщується по вертикалі,
    // враховуючи масштаб (пікселі/одиниця) і центральне зміщення
    float minVal = offset_y - (area.height / 2) / scale;
    float maxVal = offset_y + (area.height / 2) / scale;

    // Мінімальна відстань між мітками, щоб текст не накладався; 1.5 висоти шрифту
    float min_label_spacing_px = font.height * 1.5f;

    // Діапазон значень, які треба відобразити на шкалі
    float value_range = maxVal - minVal;

    // Обчислюємо скільки міток оптимально вкладеться по висоті з таким відступом
    float labels_count = area.height / min_label_spacing_px;

    // Сирий крок міток у значеннях, який потім округлюємо для зручності
    float step_raw = value_range / labels_count;
    float step;

    // Округлення кроку до зручних значень для полегшення читання шкали
    if (step_raw <= 25.0f) step = 25.0f;
    else if (step_raw <= 50.0f) step = 50.0f;
    else if (step_raw <= 100.0f) step = 100.0f;
    else step = 200.0f;

    // Малюємо основну вертикальну лінію шкали праворуч в межах області
    DrawLine(x_end, area.y, x_end, area.y + area.height, color);

    // Цикл по значеннях від найближчого кратного кроку до мінімального значення
    // до максимального, з інкрементом кроку step
    for (float val = ((int)(minVal / step)) * step; val <= maxVal; val += step)
    {
        // Обчислюємо Y позицію для мітки, масштабуючи відносно offset_y
        float y = centerY + (val - offset_y) * scale;

        // Інвертований напрямок шкали по осі Y
        // float y = centerY - (val - offset_y) * scale;

        // Якщо позиція виходить за межі області малювання - пропускаємо
        if (y < area.y || y > area.y + area.height) continue;

        // Малюємо риску довжиною 10 пікселів ліворуч від вертикальної лінії
        DrawLine(x_end, y, x_end - 10, y, color);

        // Формуємо рядок тексту
        char label[16];
        snprintf(label, sizeof(label), "%.0f", val);

        // Обчислюємо ширину тексту (припустимо, ширина символу приблизно font.width)
        int text_width = utf8_strlen(label) * font.width;

        // Виводимо числову мітку (ціле число) ліворуч від риски з відступом spacing
        DrawPSFText(font, x_end - 35 + spacing - text_width / 2, y - font.height / 2, label, spacing, color);
    }
}

