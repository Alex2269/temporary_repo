// guicheckbox.h

#ifndef GUICHECKBOX_H
#define GUICHECKBOX_H

#include "raylib.h"
#include "psf_font.h"      // Заголовок із парсером PSF-шрифту

// Оголошення функцій для малювання та обробки натискання кожної кнопки
void Gui_CheckBox(Rectangle bounds, bool *checked, PSF_Font font, const char *textTop, const char *textRight, Color color);

#endif // GUICHECKBOX_H

