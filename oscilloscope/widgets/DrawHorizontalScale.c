#include "DrawHorizontalScale.h"
#include <stdio.h>

void DrawHorizontalScale(int channel, float scale, float offset_x, Rectangle area, RasterFont font, Color color)
{
    int spacing = 2; // Відступ тексту від рисок шкали

    int y_end = area.y + area.height; // Нижня межа області для шкали
    float centerX = area.x + area.width / 2.0f; // Центр області по горизонталі

    float margin = font.glyph_height; // Відступ зліва і справа для обмеження міток і тексту
    float x_min = area.x + margin;             // Ліва межа малювання
    float x_max = area.x + area.width - margin; // Права межа малювання

    // Мінімальне і максимальне значення шкали відповідно до ширини, масштабу і зміщення offset_x
    float minVal = offset_x - ((area.width / 2.0f - margin) / scale);
    float maxVal = offset_x + ((area.width / 2.0f - margin) / scale);

    // Мінімальна відстань між мітками в пікселях, щоб текст не накладався
    float min_label_spacing_px = font.glyph_height * 2.5f;

    float value_range = maxVal - minVal;

    float labels_count = area.width / min_label_spacing_px;

    // Округлений крок міток для рівномірного розміщення (тут крок у значеннях)
    float step = 50.0f / scale;

    // Малюємо основну горизонтальну лінію шкали в нижній частині області
    DrawLine(area.x, y_end, area.x + area.width, y_end, color);

    for (float val = ((int)(minVal / step)) * step; val <= maxVal; val += step)
    {
        // Обчислення X позиції для мітки (інверсія включена)
        float x = centerX + (val - offset_x) * scale;

        if (x < x_min || x > x_max) continue;

        // Формуємо текст для мітки
        char label[16];
        snprintf(label, sizeof(label), "%.0f", val);

        int text_width = utf8_strlen(label) * font.glyph_width;

        // Малюємо риску зверху від основної лінії
        DrawLine(x, y_end, x, y_end - 10, color);

        // Малюємо текст над рискою, центруючи по Х
        DrawTextScaled(font,
                       x - text_width / 2,
                       y_end - font.glyph_height - 15,
                       label, spacing, 1, color);
    }
}

