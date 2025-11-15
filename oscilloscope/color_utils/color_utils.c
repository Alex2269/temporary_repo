// color_utils.c

#include "color_utils.h"

// Допоміжна функція для конвертації RGB (uint32_t) у компоненти float 0..1
// наразі не використовується
void RGBtoFloatComponents(uint32_t color, float *r, float *g, float *b) {
    *r = ((color >> 16) & 0xFF) / 255.0f;
    *g = ((color >> 8) & 0xFF) / 255.0f;
    *b = (color & 0xFF) / 255.0f;
}

// Допоміжна функція для конвертації float-компонент 0..1 у uint32_t RGB
// наразі не використовується
uint32_t FloatComponentsToRGB(float r, float g, float b) {
    uint32_t R = (uint32_t)(r * 255.0f) & 0xFF;
    uint32_t G = (uint32_t)(g * 255.0f) & 0xFF;
    uint32_t B = (uint32_t)(b * 255.0f) & 0xFF;
    return (R << 16) | (G << 8) | B;
}

// Обчислення яскравості кольору (luminance), потрібно для вибору контрасту
float GetLuminance(Color color) {
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Вибір білого або чорного кольору, щоб текст був контрастним до фону
Color GetContrastColor(Color color) {
    return (GetLuminance(color) > 0.5f) ? BLACK : WHITE;
}

// Зміна насиченості (saturation) кольору у HSV просторі
Color ChangeSaturation(Color c, float saturationScale) {
    float r = c.r / 255.0f;
    float g = c.g / 255.0f;
    float b = c.b / 255.0f;

    float cMax = fmaxf(r, fmaxf(g, b));
    float cMin = fminf(r, fminf(g, b));
    float delta = cMax - cMin;

    float h = 0.0f;
    if (delta > 0) {
        if (cMax == r)
            h = fmodf((g - b) / delta, 6.0f);
        else if (cMax == g)
            h = (b - r) / delta + 2.0f;
        else
            h = (r - g) / delta + 4.0f;
        h *= 60.0f;
        if (h < 0) h += 360.0f;
    }
    float s = (cMax == 0) ? 0 : delta / cMax;
    float v = cMax;

    s *= saturationScale;
    if (s > 1.0f) s = 1.0f;

    float cVal = v * s;
    float xVal = cVal * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - cVal;

    Color newColor = { 0 };
    if (h >= 0 && h < 60) {
        newColor.r = (cVal + m) * 255;
        newColor.g = (xVal + m) * 255;
        newColor.b = (0 + m) * 255;
    } else if (h >= 60 && h < 120) {
        newColor.r = (xVal + m) * 255;
        newColor.g = (cVal + m) * 255;
        newColor.b = (0 + m) * 255;
    } else if (h >= 120 && h < 180) {
        newColor.r = (0 + m) * 255;
        newColor.g = (cVal + m) * 255;
        newColor.b = (xVal + m) * 255;
    } else if (h >= 180 && h < 240) {
        newColor.r = (0 + m) * 255;
        newColor.g = (xVal + m) * 255;
        newColor.b = (cVal + m) * 255;
    } else if (h >= 240 && h < 300) {
        newColor.r = (xVal + m) * 255;
        newColor.g = (0 + m) * 255;
        newColor.b = (cVal + m) * 255;
    } else if (h >= 300 && h < 360) {
        newColor.r = (cVal + m) * 255;
        newColor.g = (0 + m) * 255;
        newColor.b = (xVal + m) * 255;
    }
    newColor.a = c.a; // Зберігаємо прозорість
    return newColor;
}

// Функція інверсії кольору (кольори RGB інвертуємо, альфа залишаємо)
Color InvertColor(Color c) {
    Color inverted;
    inverted.r = 255 - c.r;
    inverted.g = 255 - c.g;
    inverted.b = 255 - c.b;
    inverted.a = c.a;
    return inverted;
}

// Перевірка, чи є колір темним (яскравість нижча за 0.5)
int IsColorDark(Color c) {
    return GetLuminance(c) < 0.5f;
}

// Отримання контрастного інверсного кольору для фону із коригуванням насиченості і яскравості
Color GetContrastInvertColor(Color color) {
    Color invColor = InvertColor(color);
    if (IsColorDark(color)) {
        // Якщо текст темний, робимо фон світлим і насиченим
        invColor = ChangeSaturation(invColor, 0.35f);
        invColor.r = (invColor.r + 255) / 2;
        invColor.g = (invColor.g + 255) / 2;
        invColor.b = (invColor.b + 255) / 2;
    } else {
        // Якщо текст світлий, фон темніший і менш насичений
        invColor = ChangeSaturation(invColor, 0.65f);
        invColor.r /= 2;
        invColor.g /= 2;
        invColor.b /= 2;
    }
    invColor.a = 255; // Фон повністю непрозорий
    return invColor;
}

