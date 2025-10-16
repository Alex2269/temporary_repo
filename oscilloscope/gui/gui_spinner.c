// gui_spinner.c
#include "gui_spinner.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h> // Для roundf()

// Структура для збереження стану утримання кнопки (автоповтор при тривалому натисканні)
typedef struct {
    bool isHeld;            // Чи наразі кнопка утримується
    double holdStartTime;   // Час початку утримання в секундах
    double lastUpdateTime;  // Час останнього оновлення (секунди)
    double accumulatedTime; // Накопичений час для повторення кроку (секунди)
} HoldState;

// Внутрішній ліміт на кількість спінерів
#define MAX_SPINNERS 16

// Внутрішні масиви для станів утримання лівих і правих кнопок спінерів
static HoldState holdLeftStates[MAX_SPINNERS] = {{0}};
static HoldState holdRightStates[MAX_SPINNERS] = {{0}};

// Отримати системний час з високою точністю (монотоне джерело часу)
static double GetSystemTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}

// Повернути контрастний колір (чорний або білий) залежно від яскравості кольору
static Color GetContrastingColor(Color c) {
    float luminance = 0.2126f * c.r / 255 + 0.7152f * c.g / 255 + 0.0722f * c.b / 255;
    return (luminance > 0.5f) ? BLACK : WHITE;
}

// Напрямок стрілки кнопок
typedef enum {
    ARROW_LEFT,
    ARROW_RIGHT
} ArrowDirection;

// Малювання трикутної стрілки у межах кнопки
static void DrawArrow(Rectangle bounds, ArrowDirection dir, Color color) {
    Vector2 center = { bounds.x + bounds.width/2.0f, bounds.y + bounds.height/2.0f };
    float size = bounds.width * 0.45f;
    Vector2 points[3];

    switch (dir) {
        case ARROW_LEFT:
            points[0] = (Vector2){center.x - size, center.y};
            points[1] = (Vector2){center.x + size, center.y + size};
            points[2] = (Vector2){center.x + size, center.y - size};
            break;
        case ARROW_RIGHT:
            points[0] = (Vector2){center.x + size, center.y};
            points[1] = (Vector2){center.x - size, center.y - size};
            points[2] = (Vector2){center.x - size, center.y + size};
            break;
    }
    DrawTriangle(points[0], points[1], points[2], color);
}

// ====================================================================================
// Допоміжні inline-функції для інкременту/декременту значення
// ====================================================================================

// Інкрементує значення на step, з урахуванням типу і максимального значення.
// Повертає true, якщо значення змінилося.
static inline bool IncrementValue(void* value, void* minVal, void* maxVal,
                                  float step, GuiSpinnerValueType valueType)
{
    bool changed = false;

    if (valueType == GUI_SPINNER_FLOAT) {
        float* v = (float*)value;
        float maxV = *(float*)maxVal;
        // Перевірка, чи не перевищує нове значення максимум
        if (*v + step <= maxV) {
            *v += step;
            changed = true;
        } else if (*v != maxV) { // Якщо вже за межами, але не дорівнює maxV, округлюємо до maxV
            *v = maxV;
            changed = true;
        }
    }
    else {
        int* v = (int*)value;
        int maxV = *(int*)maxVal;
        int stepInt = (int)(step + 0.5f); // округлення float-кроку до int
        // Перевірка, чи не перевищує нове значення максимум
        if (*v + stepInt <= maxV) {
            *v += stepInt;
            changed = true;
        } else if (*v != maxV) { // Якщо вже за межами, але не дорівнює maxV, округлюємо до maxV
            *v = maxV;
            changed = true;
        }
    }

    return changed;
}

// Декрементує значення на step, з урахуванням типу і мінімального значення.
// Повертає true, якщо значення змінилося.
static inline bool DecrementValue(void* value, void* minVal, void* maxVal,
                                  float step, GuiSpinnerValueType valueType)
{
    bool changed = false;

    if (valueType == GUI_SPINNER_FLOAT) {
        float* v = (float*)value;
        float minV = *(float*)minVal;
        // Перевірка, чи не менше нове значення мінімуму
        if (*v - step >= minV) {
            *v -= step;
            changed = true;
        } else if (*v != minV) { // Якщо вже за межами, але не дорівнює minV, округлюємо до minV
            *v = minV;
            changed = true;
        }
    }
    else {
        int* v = (int*)value;
        int minV = *(int*)minVal;
        int stepInt = (int)(step + 0.5f); // округлення float-кроку до int
        // Перевірка, чи не менше нове значення мінімуму
        if (*v - stepInt >= minV) {
            *v -= stepInt;
            changed = true;
        } else if (*v != minV) { // Якщо вже за межами, але не дорівнює minV, округлюємо до minV
            *v = minV;
            changed = true;
        }
    }

    return changed;
}

// ====================================================================================

// Кнопка зі стрілкою з логікою натискання та утримання
// Оновлює *value залежно від натискання та типу значення
static bool ArrowButton(Rectangle bounds, ArrowDirection dir,
                        void* value, void* minVal, void* maxVal,
                        float step, GuiSpinnerValueType valueType,
                        HoldState* holdState, Color baseColor)
{
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds);
    bool changed = false; // Змінна для відстеження, чи змінилося значення

    // Визначаємо колір кнопки в залежності від стану миші
    Color btnColor = baseColor;
    if (mouseOver) btnColor = Fade(baseColor, 0.8f);
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnColor = Fade(baseColor, 0.6f);

    Color borderColor = GetContrastingColor(btnColor);
    const int borderThickness = 2;

    // Малюємо кнопку
    DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
                                     bounds.width + 2 * borderThickness, bounds.height + 2 * borderThickness},
                                     borderThickness, borderColor);
    DrawRectangleRec(bounds, btnColor);
    DrawArrow(bounds, dir, GetContrastingColor(btnColor));

    double now = GetSystemTime();

    // Якщо кнопка не утримується, ініціалізуємо часові змінні
    if (!holdState->isHeld) {
        holdState->lastUpdateTime = now;
        holdState->accumulatedTime = 0.0;
    }

    double deltaTime = now - holdState->lastUpdateTime;
    holdState->lastUpdateTime = now;

    bool mousePressed = mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseDown = mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    // Параметри для затримки і прискорення автоповтору при утриманні кнопки
    const double delayBeforeAccel = 0.175; // Затримка перед початком прискорення
    const double baseInterval = 0.25;      // Базовий інтервал між повторами
    const double minInterval = 0.005;      // Мінімальний інтервал при максимальному прискоренні
    const double accelRate = 0.075;        // Швидкість прискорення інтервалу

    if (mousePressed) {
        holdState->isHeld = true;
        holdState->holdStartTime = now;
        holdState->accumulatedTime = 0.0;

        // Виконуємо перший крок зміни відповідно до напряму стрілки
        if (dir == ARROW_RIGHT)
            changed = IncrementValue(value, minVal, maxVal, step, valueType);
        else
            changed = DecrementValue(value, minVal, maxVal, step, valueType);
    }
    else if (mouseDown && holdState->isHeld) {
        // Логіка автоповтору при утриманні кнопки із прискоренням
        double holdDuration = now - holdState->holdStartTime;
        double interval = baseInterval;

        if (holdDuration > delayBeforeAccel) {
            // Зменшуємо інтервал для прискорення
            double accelTime = holdDuration - delayBeforeAccel;
            interval = baseInterval - accelTime * accelRate;
            if (interval < minInterval) interval = minInterval; // Обмежуємо мінімальним інтервалом
        }

        holdState->accumulatedTime += deltaTime;

        // Виконуємо кроки, поки накопичений час перевищує інтервал
        while (holdState->accumulatedTime >= interval) {
            holdState->accumulatedTime -= interval;
            if (dir == ARROW_RIGHT)
                changed = IncrementValue(value, minVal, maxVal, step, valueType) || changed;
            else
                changed = DecrementValue(value, minVal, maxVal, step, valueType) || changed;
        }
    } else {
        // Кнопка відпущена, скидаємо стан утримання
        holdState->isHeld = false;
        holdState->accumulatedTime = 0.0;
    }

    return changed;
}

// Функція малювання слайдера з рівнем заповнення normPos (0..1)
static void DrawSlider(Rectangle bounds, float normPos, Color baseColor) {
    DrawRectangleRec(bounds, Fade(baseColor, 0.25f)); // Фон слайдера (прозорий)
    DrawRectangle(bounds.x, bounds.y, normPos * bounds.width, bounds.height, Fade(baseColor, 0.5f)); // Заповнення
    // DrawRectangleLinesEx(bounds, 1, GetContrastingColor(baseColor)); // Рамка

    // Ручка слайдера
    float knobX = bounds.x + normPos * bounds.width;
    float knobW = 4, knobH = bounds.height;
    Rectangle knobRect = {knobX - knobW / 2, bounds.y, knobW, knobH};
    // DrawRectangleRec(knobRect, GetContrastingColor(baseColor));
    // DrawRectangleLinesEx(knobRect, 1, baseColor);
}

// Оновлення значення за положенням миші щодо bounds слайдера (нормалізує і обмежує),
// з урахуванням кроку step для дискретизації
static void UpdateSliderValue(Rectangle bounds, void* value, void* minVal, void* maxVal,
                              float step, GuiSpinnerValueType valueType)
{
    Vector2 mousePos = GetMousePosition();
    float norm = (mousePos.x - bounds.x) / bounds.width; // Нормалізоване положення миші відносно слайдера
    if (norm < 0) norm = 0; // Обмеження в межах 0..1
    if (norm > 1) norm = 1;

    if (valueType == GUI_SPINNER_FLOAT) {
        float minV = *(float*)minVal;
        float maxV = *(float*)maxVal;
        float range = maxV - minV;
        float rawVal = minV + norm * range; // Розрахунок "сирого" значення на основі положення

        // Використання roundf для точного округлення до кратного кроку
        float steppedVal = minV + step * roundf((rawVal - minV) / step);

        // Обмеження в межах [minV, maxV]
        if (steppedVal < minV) steppedVal = minV;
        if (steppedVal > maxV) steppedVal = maxV;

        *(float*)value = steppedVal;
    } else {
        int minV = *(int*)minVal;
        int maxV = *(int*)maxVal;
        int range = maxV - minV;

        // Розрахунок цілочисельного значення з кроком
        int rawVal = minV + (int)(norm * range);

        int stepInt = (int)(step + 0.5f); // Перетворюємо float крок в int для цілочисельного типу
        if (stepInt == 0) stepInt = 1; // Запобігання ділення на нуль, якщо step був 0

        // Округлення до найближчої кратності кроку
        // (rawVal - minV) дає зміщення від мінімуму, потім округлюємо до кроку і додаємо назад minV
        int steppedVal = minV + ((rawVal - minV + stepInt/2) / stepInt) * stepInt;

        // Обмеження у межах [minV, maxV]
        if (steppedVal < minV) steppedVal = minV;
        if (steppedVal > maxV) steppedVal = maxV;

        *(int*)value = steppedVal;
    }
}

// Структура для збереження стану окремого слайдера
typedef struct {
    Rectangle bounds; // Позиція слайдера на екрані
    bool isActive;    // Чи активно (утримується) перетягування слайдера
    bool used;        // Чи використовується цей елемент (для відстеження активних спінерів)
} SliderState;

// Статичний масив для зберігання станів слайдерів за їхнім ID
static SliderState slidersState[MAX_SPINNERS] = {{0}};

// Пошук або ініціалізація стану слайдера за прямокутником bounds
// Повертає вказівник на булеву змінну isActive для цього слайдера
static bool* GetSliderActiveState(Rectangle bounds) {
    for (int i = 0; i < MAX_SPINNERS; i++) {
        if (slidersState[i].used) {
            // Якщо слайдер вже використовується і має ті ж самі межі, повертаємо його стан
            if (memcmp(&slidersState[i].bounds, &bounds, sizeof(Rectangle)) == 0) {
                return &slidersState[i].isActive;
            }
        } else {
            // Якщо знайдено вільний слот, ініціалізуємо його і повертаємо стан
            slidersState[i].bounds = bounds;
            slidersState[i].isActive = false;
            slidersState[i].used = true;
            return &slidersState[i].isActive;
        }
    }
    // Помилка: перевищено максимальну кількість спінерів
    return NULL;
}

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
                 Color baseColor, RasterFont font, int spacing)
{
    // Перевірка на валідність ID та наявності вказівника на значення
    if (id < 0 || id >= MAX_SPINNERS || !value) return false;

    // Отримання станів утримання для лівої та правої кнопок за ID
    HoldState* holdLeft = &holdLeftStates[id];
    HoldState* holdRight = &holdRightStates[id];

    bool changed = false; // Прапорець, що вказує, чи змінилося значення спінера
    int btnSize = height; // Розмір кнопок дорівнює висоті спінера
    int sliderWidth = width - 2 * btnSize; // Ширина центрального слайдера
    int posY = centerY - height / 2;       // Верхня Y-координата спінера

    // Визначення прямокутних областей для лівої кнопки, правої кнопки та слайдера
    Rectangle leftBtn = {(float)(centerX - width / 2), (float)posY, (float)btnSize, (float)height};
    Rectangle rightBtn = {(float)(centerX + width / 2 - btnSize), (float)posY, (float)btnSize, (float)height};
    Rectangle sliderRect = {(float)(leftBtn.x + btnSize), (float)posY, (float)sliderWidth, (float)height};

    // Обробка кліків по кнопках зі стрілками
    if (ArrowButton(leftBtn, ARROW_LEFT, value, minValue, maxValue,
                    step, valueType, holdLeft, baseColor)) changed = true;
    if (ArrowButton(rightBtn, ARROW_RIGHT, value, minValue, maxValue,
                    step, valueType, holdRight, baseColor)) changed = true;

    // Визначення нормалізованого значення (0..1) для відображення положення на слайдері
    float normVal = 0.0f;
    if (valueType == GUI_SPINNER_FLOAT) {
        float v = *(float*)value;
        float minV = *(float*)minValue;
        float maxV = *(float*)maxValue;
        normVal = (maxV == minV) ? 0.0f : (v - minV) / (maxV - minV);
    } else {
        int v = *(int*)value;
        int minV = *(int*)minValue;
        int maxV = *(int*)maxValue;
        normVal = (maxV == minV) ? 0.0f : ((float)(v - minV) / (float)(maxV - minV));
    }
    // Обмеження нормалізованого значення в діапазоні [0, 1]
    if (normVal < 0) normVal = 0;
    if (normVal > 1) normVal = 1;

    DrawSlider(sliderRect, normVal, baseColor); // Малювання самого слайдера

    // Отримуємо внутрішній стан "перетягування" для слайдера
    bool *isActive = GetSliderActiveState(sliderRect);
    if (!isActive) return false; // Захист від перевищення MAX_SPINNERS

    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePos, sliderRect); // Чи курсор миші над слайдером

    // Зміна значення колесом миші при наведенні на слайдер
    if (mouseOver) {
        int wheelMove = GetMouseWheelMove(); // Отримуємо рух колеса миші (вгору/вниз)
        if (wheelMove != 0) {
            // Змінюємо значення на крок * кількість "кліків" колеса
            if (valueType == GUI_SPINNER_FLOAT) {
                float *v = (float*)value;
                float maxV = *(float*)maxValue;
                float minV = *(float*)minValue;
                *v += step * wheelMove; // Зміна значення
                // Обмеження значення в діапазоні minV..maxV
                if (*v > maxV) *v = maxV;
                if (*v < minV) *v = minV;
                changed = true; // Значення змінилося
            } else {
                int *v = (int*)value;
                int maxV = *(int*)maxValue;
                int minV = *(int*)minValue;
                *v += (int)(step) * wheelMove; // Зміна значення
                // Обмеження значення в діапазоні minV..maxV
                if (*v > maxV) *v = maxV;
                if (*v < minV) *v = minV;
                changed = true; // Значення змінилося
            }
        }
    }

    // Початок перетягування слайдера
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver) {
        *isActive = true; // Активуємо стан перетягування
    }
    // Кінець перетягування
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        *isActive = false; // Деактивуємо стан перетягування
    }

    // Якщо слайдер тягнеться — оновлюємо значення
    if (*isActive) {
        // Запам'ятовуємо старе значення, щоб визначити, чи дійсно воно змінилося
        bool valueChanged = false;
        if (valueType == GUI_SPINNER_FLOAT) {
            float oldVal = *(float*)value;
            UpdateSliderValue(sliderRect, value, minValue, maxValue, step, valueType);
            if (oldVal != *(float*)value) valueChanged = true;
        } else {
            int oldVal = *(int*)value;
            UpdateSliderValue(sliderRect, value, minValue, maxValue, step, valueType);
            if (oldVal != *(int*)value) valueChanged = true;
        }
        if (valueChanged) changed = true; // Якщо значення змінилося, встановлюємо прапорець
    }

    // ==============================================================================
    // Малювання текстів над кнопками (якщо вони задані)
    // ==============================================================================
    int pad = 6; // Відступ для тексту всередині рамки

    // Текст над лівою кнопкою
    if (textLeft && textLeft[0]) {
        // Підрахунок кількості кодових точок UTF-8 для правильного виміру ширини тексту
        int len = 0;
        for (const char *p = textLeft; *p; p++) if (((*p)&0xC0)!=0x80) len++;

        float textWidth = len * (font.glyph_width + spacing) - spacing;
        float boxW = textWidth + 2*pad;
        float boxH = font.glyph_height + 2*pad;

        // Позиція рамки з текстом над лівою кнопкою
        Rectangle rLeft = {
            leftBtn.x + leftBtn.width/2.0f - boxW/2.0f,
            leftBtn.y - boxH - 8, // Відступ вгору від кнопки
            boxW,
            boxH
        };
        Color bg = Fade(GetContrastingColor(baseColor), 0.9f); // Фон рамки
        Color fg = GetContrastingColor(bg); // Колір тексту
        DrawRectangleRec(rLeft, bg);
        DrawRectangleLinesEx(rLeft, 1, fg);
        DrawTextScaled(font, (int)(rLeft.x + pad), (int)(rLeft.y + pad/2), textLeft, spacing, 1, fg);
    }

    // Текст над правою кнопкою
    if (textRight && textRight[0]) {
        // Підрахунок кількості кодових точок UTF-8
        int len = 0;
        for (const char *p = textRight; *p; p++) if (((*p)&0xC0)!=0x80) len++;

        float textWidth = len * (font.glyph_width + spacing) - spacing;
        float boxW = textWidth + 2*pad;
        float boxH = font.glyph_height + 2*pad;

        // Позиція рамки з текстом над правою кнопкою
        Rectangle rRight = {
            rightBtn.x + rightBtn.width/2.0f - boxW/2.0f,
            rightBtn.y - boxH - 8,
            boxW,
            boxH
        };
        Color bg = Fade(GetContrastingColor(baseColor), 0.9f);
        Color fg = GetContrastingColor(bg);
        DrawRectangleRec(rRight, bg);
        DrawRectangleLinesEx(rRight, 1, fg);
        DrawTextScaled(font, (int)(rRight.x + pad), (int)(rRight.y + pad/2), textRight, spacing, 1, fg);
    }

    // ==============================================================================
    // Відображення текстового рядка з поточним значенням у центрі слайдера
    // ==============================================================================
    char valStr[32]; // Буфер для форматованого значення
    if(valueType == GUI_SPINNER_FLOAT)
        snprintf(valStr, sizeof(valStr), "%.2f", *(float*)value); // Форматування float
    else
        snprintf(valStr, sizeof(valStr), "%d", *(int*)value);     // Форматування int

    // Обчислюємо ширину і висоту тексту у пікселях для центрування
    int textLen = 0;
    for(const char* p = valStr; *p; p++) if(((*p) & 0xC0) != 0x80) textLen++;
    int textWidth = textLen * (font.glyph_width + spacing) - spacing;
    int textHeight = font.glyph_height;

    // Центр слайдера по X та Y для позиціонування тексту
    centerX = sliderRect.x + sliderRect.width / 2.0f;
    centerY = sliderRect.y + sliderRect.height / 2.0f;

    // Відступи навколо тексту в прямокутнику фону (padding)
    const int padX = 8;
    const int padY = 4;

    // Обчислюємо координати прямокутника фону так, щоб текст лежав по центру слайдера
    Rectangle valBgRect = {
        centerX - (textWidth / 2.0f) - padX,
        centerY - (textHeight / 2.0f) - padY,
        textWidth + 2*padX,
        textHeight + 2*padY
    };

    // Колір фону для відображення значення з напівпрозорістю
    Color valBgColor = Fade(baseColor, 0.2f);
    Color valFgColor = GetContrastingColor(valBgColor); // Контрастний колір для тексту

    // Малюємо фон-прямокутник
    // DrawRectangleRec(valBgRect, valBgColor);
    // Малюємо рамку навколо прямокутника (контрастною)
    // DrawRectangleLinesEx(valBgRect, 1, valFgColor);

    // Малюємо текст, спроєктований у центр прямокутника
    int textDrawX = (int)(valBgRect.x + padX);
    int textDrawY = (int)(valBgRect.y + padY);

    DrawTextScaled(font, textDrawX, textDrawY, valStr, spacing, 1, valFgColor);

    return changed; // Повертаємо true, якщо значення спінера змінилося
}

