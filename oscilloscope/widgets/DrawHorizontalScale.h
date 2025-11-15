#ifndef DRAWHORIZONTALSCALE_H
#define DRAWHORIZONTALSCALE_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

void DrawHorizontalScale(int channel, float scale, float offset_x, Rectangle area, RasterFont font, Color color);

#endif // DRAWHORIZONTALSCALE_H
