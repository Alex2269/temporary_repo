// file DrawVerticalScale.h

#ifndef DRAWVERTICALSCALE_H
#define DRAWVERTICALSCALE_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

void DrawVerticalScale(int channel, float scale, float offset_y, Rectangle area, RasterFont font, Color color);

#endif // DRAWVERTICALSCALE_H

