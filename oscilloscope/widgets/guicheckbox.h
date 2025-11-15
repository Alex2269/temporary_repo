// guicheckbox.h

#ifndef GUICHECKBOX_H
#define GUICHECKBOX_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

// Оголошення функцій для малювання та обробки натискання кожної кнопки
void Gui_CheckBox(Rectangle bounds, bool *checked, RasterFont font, const char *textTop, const char *textRight, Color color);

#endif // GUICHECKBOX_H

