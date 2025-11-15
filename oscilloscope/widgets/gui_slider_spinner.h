#ifndef GUI_SPINNER_H
#define GUI_SPINNER_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

#define MAX_SPINNERS 16      // Максимальна кількість спінерів, які можуть одночасно існувати

// Типи значення, яке може зберігатися у спінері: int або float
typedef enum {
    GUI_SPINNER_INT,
    GUI_SPINNER_FLOAT
} GuiSpinnerValueType;

// Орієнтація спінера: горизонтальна або вертикальна
typedef enum {
    GUI_SPINNER_HORIZONTAL,
    GUI_SPINNER_VERTICAL
} GuiSpinnerOrientation;

// Напрямок стрілок на кнопках
typedef enum {
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
} ArrowDirection;

// Структура для зберігання стану утримання кнопки з автоповтором
typedef struct {
    bool isHeld;            // Чи кнопка утримується зараз (натиснута і не відпущена)
    double holdStartTime;   // Час, коли почалося утримання (у секундах)
    double lastUpdateTime;  // Час останнього оновлення утримання (у секундах)
    double accumulatedTime; // Накопичений час для автоповтору (у секундах)
} HoldState;

/**
 * Поєднаний віджет слайдера та спінера з опціональними кнопками.
 *
 * @param id - унікальний ідентифікатор віджета для збереження стану.
 * @param centerX, centerY - координати центру віджета на екрані.
 * @param width, height - розміри віджета по горизонталі і вертикалі.
 * @param textLeft, textRight - підписи над відповідними кнопками або під слайдером (можуть бути NULL).
 * @param value - вказівник на int або float значення, яке віджет буде відображати і змінювати.
 * @param minValue, maxValue - вказівники на мінімальне і максимальне припустиме значення.
 * @param step - крок зміни значення при натисканні кнопок або рухові слайдера.
 * @param valueType - тип значення: GUI_SPINNER_INT або GUI_SPINNER_FLOAT.
 * @param orientation - орієнтація віджета: горизонтальна або вертикальна.
 * @param baseColor - базовий колір віджета.
 * @param font - шрифт, який використовується для підписів.
 * @param spacing - відстань між символами у підписах.
 * @param showButtons - прапорець, чи відображати кнопки інкременту/декременту (true/false).
 *
 * @return true, якщо значення змінилося після взаємодії користувача, інакше false.
 */
bool Gui_SliderSpinner(int id, int centerX, int centerY, int width, int height,
                       const char* textLeft, const char* textRight,
                       void* value, void* minValue, void* maxValue,
                       float step, GuiSpinnerValueType valueType,
                       GuiSpinnerOrientation orientation,
                       Color baseColor, RasterFont font, int spacing,
                       bool showButtons);

#endif // GUI_SPINNER_H
