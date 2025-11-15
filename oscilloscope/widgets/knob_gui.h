// knob_gui.h

#ifndef KNOB_GUI_H
#define KNOB_GUI_H

#include "raylib.h"
#include <stdbool.h>

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

/**
 * @brief Відображає та обробляє поворотний регулятор (knob) для певного каналу.
 *
 * @param channel Номер каналу (індекс від 0 до CHANNEL_COUNT-1)
 * @param font_knob Шрифт для числової шкали регулятора
 * @param font_value Шрифт для відображення значення регулятора
 * @param x_pos Координата X центру регулятора на екрані
 * @param y_pos Координата Y центру регулятора на екрані
 * @param textTop Текст підказки, що відображається зверху при наведенні
 * @param textRight Текст праворуч регулятора
 * @param radius Радіус кола регулятора в пікселях
 * @param value Вказівник на значення регулятора типу float (в межах minValue..maxValue)
 * @param minValue Мінімальне значення регулятора (float)
 * @param maxValue Максимальне значення регулятора (float)
 * @param isActive Чи активний регулятор (чи реагує на події користувача)
 * @param colorText Колір тексту значення регулятора (тип Color з raylib)
 * @return int 1, якщо значення регулятора змінилося, інакше 0.
 */
int Gui_Knob_Channel(int channel, RasterFont font_knob, RasterFont font_value,
                     int x_pos, int y_pos, const char* textTop, const char* textRight,
                     float radius, float* value, float minValue, float maxValue,
                     bool isActive, Color colorText);

#endif // KNOB_GUI_H

