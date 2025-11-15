// gui_radiobutton_row.h

#ifndef GUI_RADIOBUTTON_ROW_H
#define GUI_RADIOBUTTON_ROW_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

/**
 * Малює ряд з радіокнопок (квадратних кнопок однакового розміру).
 *
 * @param bounds - прямокутник для всієї групи (ширина = itemCount*buttonSize + (itemCount-1)*spacing)
 * @param items - масив рядків назв кнопок
 * @param itemCount - кількість кнопок (наприклад, 3)
 * @param currentIndex - індекс поточного вибору
 * @param colorActive - колір активної кнопки
 * @param buttonSize - розмір квадратної кнопки (ширина і висота)
 * @param spacing - відступ між кнопками
 *
 * @return новий індекс вибраної кнопки
 */
int Gui_RadioButtons_Row(Rectangle bounds, RasterFont font, const char **items, int itemCount, int currentIndex, Color colorActive, int buttonSize, int spacing);

#endif // GUI_RADIOBUTTON_ROW_H

