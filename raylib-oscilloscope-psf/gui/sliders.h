// sliders.h

#ifndef SLIDERS_H
#define SLIDERS_H

#include "raylib.h"
#include "psf_font.h"      // Заголовок із парсером PSF-шрифту

// Функція, що малює слайдер, обробляє введення і повертає оновлене значення
// minValue, maxValue — діапазон значень слайдера
// value — поточне значення (передається і повертається)
// isVertical — орієнтація слайдера
float Gui_Slider(Rectangle bounds, PSF_Font font, const char *textTop, const char *textRight,
                 float *value, float minValue, float maxValue, bool isVertical, Color colorText);

#endif // SLIDERS_H

