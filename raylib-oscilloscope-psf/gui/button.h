// button.h

#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"
#include "psf_font.h"      // Заголовок із парсером PSF-шрифту

// Функція кнопки з автоматичним підбором кольору тексту
bool Gui_Button(Rectangle bounds, PSF_Font font, const char *text,
                Color colorNormal, Color colorHover, Color colorPressed, Color colorText);

#endif // BUTTON_H

