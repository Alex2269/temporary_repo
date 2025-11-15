// button.h

#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

// Функція кнопки з автоматичним підбором кольору тексту
bool Gui_Button(Rectangle bounds, RasterFont font, const char *text,
                Color colorNormal, Color colorHover, Color colorPressed, Color colorText);

#endif // BUTTON_H

