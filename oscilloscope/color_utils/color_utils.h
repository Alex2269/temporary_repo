// color_utils.h

#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include "raylib.h"
#include <stdint.h>
#include <math.h>


// Допоміжна функція для конвертації RGB (uint32_t) у компоненти float 0..1
// наразі не використовується
void RGBtoFloatComponents(uint32_t color, float *r, float *g, float *b);
// Допоміжна функція для конвертації float-компонент 0..1 у uint32_t RGB
// наразі не використовується
uint32_t FloatComponentsToRGB(float r, float g, float b);

// Обчислення яскравості кольору (luminance), потрібно для вибору контрасту
float GetLuminance(Color color);
// Вибір білого або чорного кольору, щоб текст був контрастним до фону
Color GetContrastColor(Color color);
// Зміна насиченості (saturation) кольору у HSV просторі
Color ChangeSaturation(Color c, float saturationScale);
// Функція інверсії кольору (кольори RGB інвертуємо, альфа залишаємо)
Color InvertColor(Color c);
// Перевірка, чи є колір темним (яскравість нижча за 0.5)
int IsColorDark(Color c);
// Отримання контрастного інверсного кольору для фону із коригуванням насиченості і яскравості
Color GetContrastInvertColor(Color color);


#endif // COLOR_UTILS_H

