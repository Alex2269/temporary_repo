// gui_slider_spinner.c
//
// Повна реалізація спінера як об'єднаного віджета слайдера з кнопками інкременту/декременту,
// що підтримує int і float, горизонтальну і вертикальну орієнтації,
// і працює коректно при мінімумі, більшому за максимум.
//

#include "gui_slider_spinner.h"   // Підключення заголовкового файлу зі структурою і прототипами
#include "color_utils.h"          // Підключення утиліт для роботи з кольорами
#include <stdio.h>
#include <string.h>
#include <time.h>                 // Для функції отримання часу
#include <math.h>                 // Для математичних операцій

// -----------------------------------------------------------------------------
// Глобальні змінні для стану утримання кнопок автоповтору

static HoldState holdLeftStates[MAX_SPINNERS] = {{0}};  // Масив станів утримання для лівих кнопок спінерів
static HoldState holdRightStates[MAX_SPINNERS] = {{0}}; // Масив станів утримання для правих кнопок спінерів

// -----------------------------------------------------------------------------
// Функції інкременту/декременту зі зворотньою логікою (підтримка min>max)

/**
 * Збільшує значення (int або float) з урахуванням допустимих меж.
 *
 * @param value - вказівник на значення для зміни.
 * @param minVal - вказівник на мінімальне допустиме значення.
 * @param maxVal - вказівник на максимальне допустиме значення.
 * @param step - крок збільшення значення.
 * @param valueType - тип значення (int або float).
 * @return true, якщо значення змінилося, інакше false.
 */
static inline bool IncrementValue(void* value, void* minVal, void* maxVal,
                                  float step, GuiSpinnerValueType valueType)
{
    bool changed = false;        // Прапорець зміни значення

    if (valueType == GUI_SPINNER_FLOAT) { // Для float типу
        float* v = (float*)value;              // Вказівник на значення
        float a = *(float*)minVal;             // Мінімальне значення
        float b = *(float*)maxVal;             // Максимальне значення
        bool reversed = (a > b);                // Прапорець "зворотньої" логіки (якщо min > max)
        float realMin = fminf(a, b);            // Реальний мінімум (нижча межа)
        float realMax = fmaxf(a, b);            // Реальний максимум (верхня межа)

        if (!reversed) {                        // Нормальна орієнтація
            if (*v + step <= realMax) {        // Якщо додаючи крок не виходимо за максимум
                *v += step;                    // Додаємо крок
                changed = true;                // Позначаємо зміну
            } else if (*v != realMax) {        // Якщо перевищення, але значення не максимум
                *v = realMax;                  // Встановлюємо максимум
                changed = true;
            }
        } else {                              // Зворотня логіка (min > max)
            // Для reversed інкремент - насправді зменшення до realMin
            if (*v - step >= realMin) {
                *v -= step;
                changed = true;
            } else if (*v != realMin) {
                *v = realMin;
                changed = true;
            }
        }
    } else {                                  // Для int типу
        int* v = (int*)value;                  // Вказівник на int-значення
        int a = *(int*)minVal;                 // Мінімум
        int b = *(int*)maxVal;                 // Максимум
        bool reversed = (a > b);               // Перевірка reversed логіки
        int realMin = (a < b) ? a : b;         // Нижня межа
        int realMax = (a > b) ? a : b;         // Верхня межа
        int stepInt = (int)(step + 0.5f);     // Крок округлений до int

        if (!reversed) {                      // Класична орієнтація
            if (*v + stepInt <= realMax) {    // Якщо збільшення не виходить за межі
                *v += stepInt;                // Збільшуємо
                changed = true;
            } else if (*v != realMax) {       // Встановлюємо максимум, якщо потрібно
                *v = realMax;
                changed = true;
            }
        } else {                            // Зворотня логіка для int
            if (*v - stepInt >= realMin) {
                *v -= stepInt;
                changed = true;
            } else if (*v != realMin) {
                *v = realMin;
                changed = true;
            }
        }
    }

    return changed;  // Повертаємо статус зміни
}

/**
 * Зменшує значення (int або float) з урахуванням допустимих меж.
 *
 * @param value - вказівник на значення.
 * @param minVal - вказівник на мінімальне значення.
 * @param maxVal - вказівник на максимальне значення.
 * @param step - крок зменшення.
 * @param valueType - тип значення.
 * @return true, якщо значення змінилося, інакше false.
 */
static inline bool DecrementValue(void* value, void* minVal, void* maxVal,
                                  float step, GuiSpinnerValueType valueType)
{
    bool changed = false;      // Прапорець зміни

    if (valueType == GUI_SPINNER_FLOAT) {
        float* v = (float*)value;
        float a = *(float*)minVal;
        float b = *(float*)maxVal;
        bool reversed = (a > b);
        float realMin = fminf(a, b);
        float realMax = fmaxf(a, b);

        if (!reversed) {
            if (*v - step >= realMin) {
                *v -= step;
                changed = true;
            } else if (*v != realMin) {
                *v = realMin;
                changed = true;
            }
        } else {
            // Для reversed декремент - це збільшення до realMax
            if (*v + step <= realMax) {
                *v += step;
                changed = true;
            } else if (*v != realMax) {
                *v = realMax;
                changed = true;
            }
        }
    } else {
        int* v = (int*)value;
        int a = *(int*)minVal;
        int b = *(int*)maxVal;
        bool reversed = (a > b);
        int realMin = (a < b) ? a : b;
        int realMax = (a > b) ? a : b;
        int stepInt = (int)(step + 0.5f);

        if (!reversed) {
            if (*v - stepInt >= realMin) {
                *v -= stepInt;
                changed = true;
            } else if (*v != realMin) {
                *v = realMin;
                changed = true;
            }
        } else {
            if (*v + stepInt <= realMax) {
                *v += stepInt;
                changed = true;
            } else if (*v != realMax) {
                *v = realMax;
                changed = true;
            }
        }
    }

    return changed;  // Повертаємо результат зміни
}

// -----------------------------------------------------------------------------
// Високоточний час для автоповтору

/**
 * Отримує системний час у секундах з високою точністю.
 * Використовується для обробки автоповтору натискань.
 *
 * @return час у секундах з плаваючою точкою.
 */
static double GetSystemTime()
{
    struct timespec ts;                    // Структура для часу
    clock_gettime(CLOCK_MONOTONIC, &ts);  // Отримуємо монотонний час
    return (double)ts.tv_sec + ts.tv_nsec / 1e9; // Конвертуємо в секунди з наносекундами
}

// -----------------------------------------------------------------------------
// Малювання стрілок

/**
 * Малює трикутну стрілку в межах заданого прямокутника.
 *
 * @param bounds - прямокутник, в якому буде намальовано стрілку.
 * @param dir - напрямок стрілки (ліворуч, праворуч, вгору, вниз).
 * @param color - колір стрілки.
 */
static void DrawArrow(Rectangle bounds, ArrowDirection dir, Color color)
{
    Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f}; // Центр прямокутника
    float size = bounds.width * 0.45f;  // Розмір стрілки як 45% від ширини
    Vector2 points[3];                  // Координати трьох вершин трикутника

    switch (dir) {                     // Визначаємо позиції вершин залежно від напрямку
        case ARROW_LEFT:
            points[0] = (Vector2){center.x + size, center.y - size};
            points[1] = (Vector2){center.x - size, center.y};
            points[2] = (Vector2){center.x + size, center.y + size};
            break;
        case ARROW_RIGHT:
            points[0] = (Vector2){center.x + size, center.y};
            points[1] = (Vector2){center.x - size, center.y - size};
            points[2] = (Vector2){center.x - size, center.y + size};
            break;
        case ARROW_DOWN:
            points[0] = (Vector2){center.x - size, center.y - size};
            points[1] = (Vector2){center.x, center.y + size};
            points[2] = (Vector2){center.x + size, center.y - size};
            break;
        case ARROW_UP:
            points[0] = (Vector2){center.x, center.y - size};
            points[1] = (Vector2){center.x - size, center.y + size};
            points[2] = (Vector2){center.x + size, center.y + size};
            break;
    }
    DrawTriangle(points[0], points[1], points[2], color); // Малюємо трикутник
}

// -----------------------------------------------------------------------------
// Кнопка-стрілка з автоповтором і прискоренням

/**
 * Малює кнопку зі стрілкою, обробляє взаємодію з мишею,
 * підтримує затримку перед автоповтором і прискорення зміни значення.
 *
 * @param bounds - область кнопки.
 * @param dir - напрям стрілки.
 * @param value - вказівник на значення.
 * @param minVal - мінімальне значення.
 * @param maxVal - максимальне значення.
 * @param step - крок зміни.
 * @param valueType - тип значення (int/float).
 * @param holdState - стан утримання кнопки.
 * @param baseColor - базовий колір кнопки.
 * @param orientation - орієнтація спінера.
 *
 * @return true, якщо значення змінилося.
 */
static bool ArrowButton(Rectangle bounds, ArrowDirection dir,
                        void* value, void* minVal, void* maxVal,
                        float step, GuiSpinnerValueType valueType,
                        HoldState* holdState, Color baseColor,
                        GuiSpinnerOrientation orientation)
{
    Vector2 mousePos = GetMousePosition();                  // Поточна позиція миші
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds); // Перевірка чи курсор над кнопкою
    bool changed = false;                                    // Прапорець зміни значення

    Color btnColor = baseColor;                              // Початковий колір кнопки
    if (mouseOver) btnColor = Fade(baseColor, 0.8f);       // Освітлення кольору при наведенні
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        btnColor = Fade(baseColor, 0.6f);                   // І ще більше затемнення при натисканні

        Color borderColor = GetContrastColor(btnColor);         // Контрастний колір для обводки
        int borderThickness = 2;                                 // Товщина обводки
        // Малюємо обводку навколо області кнопки з відступом borderThickness
        DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
            bounds.width + 2 * borderThickness, bounds.height + 2 * borderThickness},
            borderThickness, borderColor);
        DrawRectangleRec(bounds, btnColor);                      // Малюємо саму кнопку

        // Визначаємо напрямок стрілки для малювання залежно від орієнтації спінера
        ArrowDirection drawDir = (orientation == GUI_SPINNER_HORIZONTAL) ?
        ((dir == ARROW_LEFT) ? ARROW_LEFT : ARROW_RIGHT) :
        (dir == ARROW_UP ? ARROW_UP : ARROW_DOWN);
        DrawArrow(bounds, drawDir, InvertColor(btnColor));       // Малюємо стрілку інвертованим кольором

        double now = GetSystemTime();                            // Поточний час для автоповтору

        if (!holdState->isHeld) {                                // Якщо кнопка раніше не утримувалась
            holdState->lastUpdateTime = now;                     // Запам'ятовуємо час останнього оновлення
            holdState->accumulatedTime = 0.0;                    // Скидаємо акумульований час
        }
        double deltaTime = now - holdState->lastUpdateTime;     // Час з останнього оновлення
        holdState->lastUpdateTime = now;                         // Оновлення часу оновлення

        bool mousePressed = mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON); // Перше натиснення кнопки
        bool mouseDown = mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON);       // Кнопка утримується

        bool isReversed = false;                                 // Прапорець зворотньої логіки
        if (valueType == GUI_SPINNER_FLOAT)
            isReversed = (*(float*)minVal > *(float*)maxVal);
    else
        isReversed = (*(int*)minVal > *(int*)maxVal);

    bool incAction = (dir == ARROW_UP || dir == ARROW_RIGHT); // Чи кнопка для інкременту

    const double delayBeforeAccel = 0.175;  // Затримка перш ніж починається прискорення
    const double baseInterval = 0.25;       // Початковий інтервал між повторними діями (в секундах)
    const double minInterval = 0.005;       // Мінімальний інтервал при максимальному прискоренні
    const double accelRate = 0.075;         // Швидкість прискорення (зменшення інтервалу)

    if (mousePressed) {                       // Якщо кнопка щойно натиснута
        holdState->isHeld = true;             // Встановлюємо стан утримання
        holdState->holdStartTime = now;       // Запам'ятовуємо час початку утримання
        holdState->accumulatedTime = 0.0;     // Скидаємо акумульований час

        if (incAction)                        // Збільшення значення
            changed = IncrementValue(value, minVal, maxVal, step, valueType);
        else                                 // Зменшення значення
            changed = DecrementValue(value, minVal, maxVal, step, valueType);
    }
    else if (mouseDown && holdState->isHeld) {  // Якщо кнопка утримується
        double holdDuration = now - holdState->holdStartTime;  // Час утримання
        double interval = baseInterval;                        // Ініціалізація інтервалу

        if (holdDuration > delayBeforeAccel) {                 // Після затримки починаємо прискорення
            double accelTime = holdDuration - delayBeforeAccel; // Час прискорення
            interval = baseInterval - accelTime * accelRate;   // Зменшення інтервалу
            if (interval < minInterval) interval = minInterval; // Обмеження мінімального інтервалу
        }

        holdState->accumulatedTime += deltaTime;               // Накопичуємо час між оновленнями

        while (holdState->accumulatedTime >= interval) {       // Якщо накопичений час перевищує інтервал
            holdState->accumulatedTime -= interval;            // Зменшуємо акумулятор

            if (incAction)                                     // Застосовуємо відповідну зміну
                changed = IncrementValue(value, minVal, maxVal, step, valueType) || changed;
            else
                changed = DecrementValue(value, minVal, maxVal, step, valueType) || changed;
        }
    }
    else {                                                   // Якщо кнопка не натиснута або відпущена
        holdState->isHeld = false;                            // Скидаємо стан утримання
        holdState->accumulatedTime = 0.0;                     // І акумульований час
    }

    return changed;  // Повертаємо інформацію про зміну значення
}

// -----------------------------------------------------------------------------
// Малювання слайдера з ручкою

/**
 * Малює слайдер з заповненням відповідно до нормалізованого положення,
 * а також ручку слайдера.
 *
 * @param bounds - область слайдера.
 * @param normPos - нормалізоване положення (0..1).
 * @param baseColor - базовий колір слайдера.
 * @param orientation - орієнтація слайдера.
 */
static void DrawSlider(Rectangle bounds, float normPos, Color baseColor, GuiSpinnerOrientation orientation)
{
    if (orientation == GUI_SPINNER_HORIZONTAL) {
        DrawRectangleRec(bounds, Fade(baseColor, 0.25f));                        // Фон слайдера
        DrawRectangle(bounds.x, bounds.y, normPos * bounds.width, bounds.height, Fade(baseColor, 0.5f)); // Заповнення
        DrawRectangleLinesEx(bounds, 1, GetContrastColor(baseColor));          // Контур слайдера

        float knobX = bounds.x + normPos * bounds.width;                       // Позиція ручки по X
        float knobW = 10;                                                     // Ширина ручки
        float knobH = bounds.height;                                          // Висота ручки
        Rectangle knobRect = {knobX - knobW / 2, bounds.y, knobW, knobH};      // Область ручки
        DrawRectangleRec(knobRect, InvertColor(baseColor));                   // Малюємо ручку (зворотнім кольором)
        DrawRectangleLinesEx(knobRect, 1, GetContrastColor(baseColor));       // Обводка ручки
    }
    else {  // Вертикальна орієнтація
        DrawRectangleRec(bounds, Fade(baseColor, 0.25f));                     // Фон слайдера
        float fillHeight = normPos * bounds.height;                           // Висота заповнення
        DrawRectangle(bounds.x, bounds.y + bounds.height - fillHeight, bounds.width, fillHeight, Fade(baseColor, 0.5f)); // Заповнення

        float knobY = bounds.y + bounds.height - fillHeight;                  // Позиція ручки по Y
        float knobW = bounds.width;                                            // Ширина ручки
        float knobH = 10;                                                      // Висота ручки
        Rectangle knobRect = {bounds.x, knobY - knobH / 2, knobW, knobH};       // Область ручки
        DrawRectangleRec(knobRect, InvertColor(baseColor));                   // Малюємо ручку
        DrawRectangleLinesEx(knobRect, 1, GetContrastColor(baseColor));       // Обводка ручки
    }
}

// -----------------------------------------------------------------------------
// Нормалізація значення в [0..1]

/**
 * Нормалізує поточне значення у відрізок [0..1] для відображення слайдера.
 * Враховує можливість перевернутого діапазону (min > max).
 *
 * @param value - поточне значення.
 * @param minVal - мінімальна межа.
 * @param maxVal - максимальна межа.
 * @param valueType - тип значення.
 *
 * @return нормалізоване значення в інтервалі [0..1].
 */
static float NormalizeValue(void* value, void* minVal, void* maxVal, GuiSpinnerValueType valueType)
{
    float normVal = 0.0f;  // Ініціалізація нормалізованого значення

    if (valueType == GUI_SPINNER_FLOAT) {                      // Для float
        float v = *(float*)value;                              // Поточне значення
        float a = *(float*)minVal;                             // Мінімум
        float b = *(float*)maxVal;                             // Максимум
        float realMin = fminf(a, b);                           // Нижня межа
        float realMax = fmaxf(a, b);                           // Верхня межа

        normVal = (realMax == realMin) ? 0.0f : (v - realMin) / (realMax - realMin); // Нормалізація
        if (a > b) normVal = 1.0f - normVal;                   // Інверсія при перевернутому діапазоні

    } else {                                                    // Для int
        int v = *(int*)value;
        int a = *(int*)minVal;
        int b = *(int*)maxVal;
        int realMin = (a < b) ? a : b;
        int realMax = (a > b) ? a : b;

        normVal = (realMax == realMin) ? 0.0f : ((float)(v - realMin) / (float)(realMax - realMin));
        if (a > b) normVal = 1.0f - normVal;                   // Інверсія при reversed
    }

    if (normVal < 0) normVal = 0;                              // Кліппінг значення у межах
    if (normVal > 1) normVal = 1;

    return normVal;                                            // Повертаємо нормалізоване значення
}

// -----------------------------------------------------------------------------
// Оновлення значення від позиції миші у слайдері

/**
 * Обчислює і встановлює нове значення на основі позиції миші всередині слайдера,
 * з урахуванням кроку зміни, типу значення і орієнтації спінера.
 *
 * @param bounds - область слайдера.
 * @param value - вказівник на значення.
 * @param minVal - мінімальне значення.
 * @param maxVal - максимальне значення.
 * @param step - крок для округлення.
 * @param valueType - тип значення (int або float).
 * @param orientation - орієнтація спінера.
 */
static void UpdateValueFromMouse(Rectangle bounds, void* value, void* minVal, void* maxVal,
                                 float step, GuiSpinnerValueType valueType, GuiSpinnerOrientation orientation)
{
    Vector2 mousePos = GetMousePosition();                      // Поточна позиція миші
    // Обчислення нормалізованого положення залежно від орієнтації (горизонталь/вертикаль)
    float norm = (orientation == GUI_SPINNER_HORIZONTAL)
    ? (mousePos.x - bounds.x) / bounds.width
    : 1.0f - (mousePos.y - bounds.y) / bounds.height;

    if (norm < 0) norm = 0;                                     // Обмеження в межах 0..1
    if (norm > 1) norm = 1;

    if (valueType == GUI_SPINNER_FLOAT) {                       // Для float
        float a = *(float*)minVal;
        float b = *(float*)maxVal;
        float realMin = fminf(a, b);
        float realMax = fmaxf(a, b);
        if (a > b) norm = 1.0f - norm;                          // Інверсія при reversed

        float range = realMax - realMin;                        // Довжина діапазону
        float rawVal = realMin + norm * range;                  // Сира позиція по діапазону
        // Округлення до найближчого кроку з врахуванням кроку step
        float steppedVal = realMin + step * roundf((rawVal - realMin) / step);

        if (steppedVal < realMin) steppedVal = realMin;         // Кліппінг
        if (steppedVal > realMax) steppedVal = realMax;

        *(float*)value = steppedVal;                             // Записуємо нове значення

    } else {                                                    // Для int
        int a = *(int*)minVal;
        int b = *(int*)maxVal;
        int realMin = (a < b) ? a : b;
        int realMax = (a > b) ? a : b;
        if (a > b) norm = 1.0f - norm;                          // Інверсія

        int range = realMax - realMin;
        int rawVal = realMin + (int)(norm * range);
        int stepInt = (int)(step + 0.5f);
        if (stepInt == 0) stepInt = 1;                           // Мінімальний крок 1

        // Округлення кроку: зсув для коректного округлення і множення назад
        int steppedVal = realMin + ((rawVal - realMin + stepInt / 2) / stepInt) * stepInt;

        if (steppedVal < realMin) steppedVal = realMin;         // Кліппінг
        if (steppedVal > realMax) steppedVal = realMax;

        *(int*)value = steppedVal;                               // Присвоєння значення
    }
}

// -----------------------------------------------------------------------------
// Стан віджету для тягнення миші

/**
 * Внутрішня структура, що зберігає стан віджету для обробки активності,
 * у тому числі, чи віджет активний і чи він використовується.
 */
typedef struct {
    Rectangle bounds;    // Область віджету
    bool isActive;       // Чи віджет зараз активний (тягнучий)
    bool used;           // Чи віджет використовується (для майбутніх цілей)
} WidgetState;

// Масив станів для максимальної кількості спінерів
static WidgetState widgetStates[MAX_SPINNERS] = {{0}};

/**
 * Повертає вказівник на прапорець активності віджету за індексом id.
 *
 * @param id - ідентифікатор спінера.
 * @return вказівник на прапорець активності, або NULL якщо id виходить за межі.
 */
static bool* GetWidgetActiveState(int id)
{
    if (id < 0 || id >= MAX_SPINNERS) return NULL;
    return &widgetStates[id].isActive;
}

// -----------------------------------------------------------------------------
// Основна функція віджету - спінер зі слайдером і кнопками

/**
 * Поєднаний віджет слайдера та спінера з опціональними кнопками.
 *
 * @param id - унікальний ідентифікатор віджета для збереження стану.
 * @param centerX, centerY - координати центру віджета.
 * @param width, height - розміри віджета.
 * @param textLeft, textRight - підписи над відповідними кнопками або під слайдером.
 * @param value - вказівник на int або float значення.
 * @param minValue, maxValue - межі для значення.
 * @param step - крок зміни значення.
 * @param valueType - тип значення (int/float).
 * @param orientation - орієнтація віджета.
 * @param baseColor - базовий колір.
 * @param font - шрифт.
 * @param spacing - відстань між символами.
 * @param showButtons - прапорець, чи відображати кнопки інкременту/декременту.
 *
 * @return true, якщо значення змінилося.
 */
bool Gui_SliderSpinner(int id, int centerX, int centerY, int width, int height,
                       const char* textLeft, const char* textRight,
                       void* value, void* minValue, void* maxValue,
                       float step, GuiSpinnerValueType valueType,
                       GuiSpinnerOrientation orientation,
                       Color baseColor, RasterFont font, int spacing,
                       bool showButtons)
{
    if (id < 0 || id >= MAX_SPINNERS || !value) return false;   // Перевірка валідності id і покажчика value

    HoldState* holdLeft = &holdLeftStates[id];      // Стан утримання лівої кнопки для цього індексу
    HoldState* holdRight = &holdRightStates[id];    // Стан утримання правої кнопки
    bool changed = false;                            // Прапорець зміни значення

    int btnSize = showButtons ? ((orientation == GUI_SPINNER_HORIZONTAL) ? height : width) : 0; // Розмір кнопок по орієнтації
    int sliderWidth = (orientation == GUI_SPINNER_HORIZONTAL) ? (width - 2 * btnSize) : width;   // Ширина слайдера
    int sliderHeight = (orientation == GUI_SPINNER_HORIZONTAL) ? height : (height - 2 * btnSize); // Висота слайдера
    int posX = centerX - width / 2;   // Ліва верхня X точка віджету
    int posY = centerY - height / 2;  // Ліва верхня Y точка

    Rectangle leftBtn, rightBtn, sliderRect;  // Області лівої і правої кнопок та слайдера

    if (orientation == GUI_SPINNER_HORIZONTAL) {
        leftBtn = (Rectangle){(float)posX, (float)posY, (float)btnSize, (float)height};                          // Ліва кнопка
        rightBtn = (Rectangle){(float)(posX + width - btnSize), (float)posY, (float)btnSize, (float)height};    // Права кнопка
        sliderRect = (Rectangle){(float)(posX + btnSize), (float)posY, (float)sliderWidth, (float)sliderHeight}; // Слайдер між кнопками
    } else {
        rightBtn = (Rectangle){(float)posX, (float)posY, (float)width, (float)btnSize};                          // Верхня кнопка
        leftBtn = (Rectangle){(float)posX, (float)(posY + height - btnSize), (float)width, (float)btnSize};     // Нижня кнопка
        sliderRect = (Rectangle){(float)posX, (float)(posY + btnSize), (float)sliderWidth, (float)sliderHeight};   // Слайдер між кнопками
    }

    bool isReversed = false;  // Перевірка перевернутого діапазону
    if (valueType == GUI_SPINNER_FLOAT)
        isReversed = (*(float*)minValue > *(float*)maxValue);
    else
        isReversed = (*(int*)minValue > *(int*)maxValue);

    if (showButtons) {   // Якщо потрібні кнопки - малюємо та обробляємо кнопки
        if (ArrowButton(leftBtn, (orientation == GUI_SPINNER_HORIZONTAL) ? ARROW_LEFT : ARROW_DOWN,
            value, minValue, maxValue,
            step, valueType, holdLeft, baseColor, orientation)) {
            changed = true;   // Якщо значення змінилось при натисканні на ліву кнопку
            }
            if (ArrowButton(rightBtn, (orientation == GUI_SPINNER_HORIZONTAL) ? ARROW_RIGHT : ARROW_UP,
                value, minValue, maxValue,
                step, valueType, holdRight, baseColor, orientation)) {
                changed = true;   // Якщо правою кнопкою змінили значення
                }
    }

    float normVal = NormalizeValue(value, minValue, maxValue, valueType);  // Нормалізуємо значення для позиції слайдера
    if (normVal < 0) normVal = 0;          // Обмеження 0..1
    if (normVal > 1) normVal = 1;

    DrawSlider(sliderRect, normVal, baseColor, orientation);  // Малюємо слайдер із ручкою

    bool* isActive = GetWidgetActiveState(id);  // Отримуємо стан активності віджету
    if (!isActive) return false;                 // Якщо нема стану - виходимо

    Vector2 mousePos = GetMousePosition();     // Позиція миші
    bool mouseOver = CheckCollisionPointRec(mousePos, sliderRect);  // Вказує чи курсор над слайдером

    if (mouseOver) {                           // Якщо курсор над слайдером
        int wheelMove = GetMouseWheelMove();  // Сприймаємо прокрутку колеса миші
        if (wheelMove != 0) {
            float delta = step * wheelMove;   // Обчислюємо зміну за кроком і кількістю прокруток
            if (valueType == GUI_SPINNER_FLOAT) {
                float* v = (float*)value;
                float a = *(float*)minValue;
                float b = *(float*)maxValue;
                *v += (a <= b) ? delta : -delta; // Інверсія зміни при reversed
                float realMin = fminf(a, b);
                float realMax = fmaxf(a, b);
                if (*v > realMax) *v = realMax;  // Обмеження по межах
                if (*v < realMin) *v = realMin;
            } else {
                int* v = (int*)value;
                int a = *(int*)minValue;
                int b = *(int*)maxValue;
                int stepInt = (int)(step + 0.5f);
                int deltaInt = (a <= b) ? stepInt * wheelMove : -stepInt * wheelMove;
                *v += deltaInt;
                int realMin = (a < b) ? a : b;
                int realMax = (a > b) ? a : b;
                if (*v > realMax) *v = realMax;
                if (*v < realMin) *v = realMin;
            }
            changed = true;   // Змінено через колесо прокрутки
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver)  // Початок тягнення слайдера мишою
        *isActive = true;
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))              // Завершення тягнення
        *isActive = false;

    if (*isActive) {  // Якщо слайдер тягнуть
        bool valueChanged = false;  // Враховуємо зміни значення
        if (valueType == GUI_SPINNER_FLOAT) {
            float oldVal = *(float*)value; // Запам'ятовуємо попереднє значення
            UpdateValueFromMouse(sliderRect, value, minValue, maxValue, step, valueType, orientation);
            if (*(float*)value != oldVal) valueChanged = true; // Визначення зміни
        } else {
            int oldVal = *(int*)value;
            UpdateValueFromMouse(sliderRect, value, minValue, maxValue, step, valueType, orientation);
            if (*(int*)value != oldVal) valueChanged = true;
        }
        if (valueChanged) changed = true;  // Позначка, що значення змінилось
    }

    // Малюємо підписи над кнопками (якщо є і якщо кнопки показуються)
    if (showButtons) {
        int CenterX, CenterY;
        int len;
        float textWidth, boxW, boxH;

        if (textRight && textRight[0]) {
            len = utf8_strlen(textRight);
            textWidth = len * (font.glyph_width + spacing) - spacing;
            boxW = textWidth;
            boxH = font.glyph_height;
            if (orientation == GUI_SPINNER_VERTICAL) {
                CenterX = (int)(rightBtn.x + rightBtn.width / 2.0f) - (int)(boxW / 2.0f);
                CenterY = (int)(rightBtn.y) - (int)(boxH + 8);
            } else {
                CenterX = (int)(rightBtn.x + rightBtn.width / 2.0f) - (int)(boxW / 2.0f);
                CenterY = (int)(rightBtn.y) - (int)(boxH + 8);
            }
            DrawTextWithAutoInvertedBackground(font, CenterX, CenterY, textRight, spacing, 1, baseColor, 2, 1);
        }
        if (textLeft && textLeft[0]) {
            len = utf8_strlen(textLeft);
            textWidth = len * (font.glyph_width + spacing) - spacing;
            boxW = textWidth;
            boxH = font.glyph_height;
            if (orientation == GUI_SPINNER_VERTICAL) {
                CenterX = (int)(leftBtn.x + leftBtn.width / 2.0f) - (int)(boxW / 2.0f);
                CenterY = (int)(leftBtn.y + leftBtn.height) + 8;
            } else {
                CenterX = (int)(leftBtn.x + leftBtn.width / 2.0f) - (int)(boxW / 2.0f);
                CenterY = (int)(leftBtn.y) - (int)(boxH + 8);
            }
            DrawTextWithAutoInvertedBackground(font, CenterX, CenterY, textLeft, spacing, 1, baseColor, 2, 1);
        }
    }

    // Малюємо підписи над та під слайдером (якщо якщо кнопоки відсутні)
    if (!showButtons) {
        int pad = 6;
        if (textLeft && textLeft[0]) {
            int len = utf8_strlen(textLeft);
            float textWidth = len * (font.glyph_width + spacing) - spacing;
            float boxW = textWidth;
            float boxH = font.glyph_height;

            int centerX, centerY;

            if (orientation == GUI_SPINNER_HORIZONTAL) {
                // В горизонтальному режимі: textLeft зліва від слайдера
                centerX = (int)(sliderRect.x) - (int)(boxW / 2.0f);
                centerY = (int)(sliderRect.y) - (int)(boxH + 8);
                // Під міткою розташовуємося внизу (під слайдером), або вправо — залежно від дизайну
                // centerY = (int)(sliderRect.y + sliderRect.height + 8);
            } else {
                // В вертикальному режимі: центр по горизонталі і внизу
                centerX = (int)(sliderRect.x + sliderRect.width / 2.0f) - (int)(boxW / 2.0f);
                centerY = (int)(sliderRect.y + sliderRect.height + 8);
            }

            DrawTextWithAutoInvertedBackground(font, centerX, centerY, textLeft, spacing, 1, baseColor, 2, 1);
        }

        if (textRight && textRight[0]) {
            int len = utf8_strlen(textRight);
            float textWidth = len * (font.glyph_width + spacing) - spacing;
            float boxW = textWidth;
            float boxH = font.glyph_height;

            int centerX, centerY;

            if (orientation == GUI_SPINNER_HORIZONTAL) {
                // Праворуч — зліва від слайдера для центрованої позиції
                centerX = (int)(sliderRect.x + sliderRect.width) - (int)(boxW / 2.0f);
                // Вгорі під передньою частиною слайдера
                centerY = (int)(sliderRect.y) - (int)(boxH + 8);
            } else {
                // В вертикальному режимі — центр по горизонталі і зверху
                centerX = (int)(sliderRect.x + sliderRect.width / 2.0f) - (int)(boxW / 2.0f);
                centerY = (int)(sliderRect.y) - (int)(boxH + 8);
            }

            DrawTextWithAutoInvertedBackground(font, centerX, centerY, textRight, spacing, 1, baseColor, 2, 1);
        }
    }

    // Вивід поточного значення числом у центрі віджета
    char valStr[32];
    if (valueType == GUI_SPINNER_FLOAT)
        snprintf(valStr, sizeof(valStr), "%.2f", *(float*)value);
    else
        snprintf(valStr, sizeof(valStr), "%d", *(int*)value);

    int textLen = 0;
    for (const char* p = valStr; *p; p++) if (((*p) & 0xC0) != 0x80) textLen++;
    int textWidth = textLen * (font.glyph_width + spacing) - spacing;
    int textHeight = font.glyph_height;

    int centerXVal = (int)(sliderRect.x + sliderRect.width / 2.0f) - (textWidth / 2);
    int centerYVal = (int)(sliderRect.y + sliderRect.height / 2.0f) - (textHeight / 2);

    DrawTextWithAutoInvertedBackground(font, centerXVal, centerYVal, valStr,
                                       spacing, 1, baseColor, 2, 1);

    return changed;  // Повертаємо інформацію про те, чи був змінений value
}

