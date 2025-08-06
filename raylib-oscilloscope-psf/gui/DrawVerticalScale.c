#include "DrawVerticalScale.h"

void DrawVerticalScale(int channel, float scale, float offset_y, Rectangle area, PSF_Font font, Color color)
{
    int spacing = 2;
    int x_end = area.x + area.width;
    float centerY = area.y + area.height / 2.0f;

    float minVal = offset_y - (area.height / 2) / scale;
    float maxVal = offset_y + (area.height / 2) / scale;

    float min_label_spacing_px = font.height * 1.5f;
    float value_range = maxVal - minVal;
    float labels_count = area.height / min_label_spacing_px;

    float step_raw = value_range / labels_count;
    float step;

    // Округлення кроку до зручного значення
    if (step_raw <= 25.0f) step = 25.0f;
    else if (step_raw <= 50.0f) step = 50.0f;
    else if (step_raw <= 100.0f) step = 100.0f;
    else step = 200.0f;

    // Малюємо основну вертикальну лінію шкали
    DrawLine(x_end, area.y, x_end, area.y + area.height, color);

    for (float val = ((int)(minVal / step)) * step; val <= maxVal; val += step)
    {
        float y = centerY + (val - offset_y) * scale;
        if (y < area.y || y > area.y + area.height) continue;

        DrawLine(x_end - 10, y, x_end, y, color);
        DrawPSFText(font, x_end - 35, y - font.height / 2,
                    TextFormat("%.0f", val), spacing, color);
    }
}

