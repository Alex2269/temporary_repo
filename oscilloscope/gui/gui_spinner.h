// gui_spinner.h
#ifndef GUI_SPINNER_H
#define GUI_SPINNER_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "glyphs.h"

// Максимальна кількість спінерів, яких підтримує бібліотека
#define MAX_SPINNERS 16

// Перелік типів даних, які може підтримувати спінер
typedef enum {
    GUI_SPINNER_INT,
    GUI_SPINNER_FLOAT
} GuiSpinnerValueType;

/**
 * Основна функція віджета спінера (з можливістю роботи з int або float).
 *
 * @param id - Ідентифікатор віджета. Дозволяє зберігати внутрішній стан по спінеру.
 * @param centerX, centerY - Координати центру віджета.
 * @param width, height - Розміри віджета.
 * @param textLeft, textRight - Текст над лівою і правою кнопками (може бути NULL).
 * @param value - Вказівник на змінну int або float, значення якої редагується.
 * @param minValue, maxValue - Вказівники на мінімальне/максимальне значення (int або float).
 * @param step - Крок зміни для натискання кнопок.
 * @param valueType - Тип значення (int чи float).
 * @param baseColor - Основний колір елемента.
 * @param font - Шрифт PSF для відображення тексту.
 * @param spacing - Відстань між символами тексту.
 *
 * @return true, якщо значення змінилося.
 */
bool Gui_Spinner(int id, int centerX, int centerY, int width, int height,
                 const char* textLeft, const char* textRight,
                 void* value, void* minValue, void* maxValue,
                 float step, GuiSpinnerValueType valueType,
                 Color baseColor, RasterFont font, int spacing);

#endif // GUI_SPINNER_H

