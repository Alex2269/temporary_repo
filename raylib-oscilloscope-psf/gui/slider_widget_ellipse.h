// slider_widget_ellipse.h
#ifndef SLIDER_WIDGET_ELLIPSE_H
#define SLIDER_WIDGET_ELLIPSE_H

#include "raylib.h"
#include "psf_font.h"

// Функції для роботи зі слайдерами з вираженими еліпсними ручками.
// Ці слайдери мають ручки у формі еліпса замість прямокутника.

// Реєструє слайдер з еліпсною ручкою або оновлює його параметри.
// sliderIndex - індекс слайдера в масиві від 0 до MAX_SLIDERS-1
// bounds - область слайдера на екрані
// value - вказівник на змінну зі значенням слайдера
// minValue, maxValue - межі значення
// isVertical - орієнтація слайдера (true - вертикальний, false - горизонтальний)
// baseColor - базовий колір слайдера
// textTop - рядок з підказкою над слайдером (може бути NULL)
// textRight - рядок з підказкою праворуч від слайдера (може бути NULL)
void RegisterEllipseKnobSlider(int sliderIndex, Rectangle bounds, float *value, float minValue, float maxValue, bool isVertical, Color baseColor, const char* textTop, const char* textRight);

// Оновлює стан усіх зареєстрованих слайдерів з еліпсними ручками і малює їх.
// Повинна викликатися один раз за кадр.
// font - шрифт для малювання тексту,
// spacing - відстань між символами
void UpdateEllipseKnobSlidersAndDraw(PSF_Font font, int spacing);

#endif // SLIDER_WIDGET_ELLIPSE_H

