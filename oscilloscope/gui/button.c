// button.c
#include "button.h"

extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

// Обчислення яскравості кольору (luminance)
static float GetLuminance(Color color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Вибір контрастного кольору тексту (білий або чорний)
static Color GetContrastingTextColor(Color bgColor)
{
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

bool Gui_Button(Rectangle bounds, RasterFont font, const char *text,
                Color colorNormal, Color colorHover, Color colorPressed, Color colorText)
{
    Vector2 mousePoint = GetMousePosition();
    bool pressed = false;

    bool mouseOver = CheckCollisionPointRec(mousePoint, bounds);
    Color btnColor = colorNormal;

    if (mouseOver) btnColor = colorHover;
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnColor = colorPressed;

    // Малюємо контур (комірець) кнопки
    Color borderColor = GetContrastingTextColor(btnColor); // контрастний колір до фону кнопки
    int borderThickness = 2;
    DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
                                    bounds.width + 2*borderThickness, bounds.height + 2*borderThickness},
                         borderThickness, borderColor);

    // Малюємо саму кнопку
    DrawRectangleRec(bounds, btnColor);

    // Визначаємо колір тексту (автоматично підбираємо, якщо альфа 0)
    Color textColor = (colorText.a == 0) ? GetContrastingTextColor(btnColor) : colorText;

    int charCount = utf8_strlen(text);
    float lineWidth = charCount * (font.glyph_width + spacing) - spacing;

    Vector2 textPos = {
        bounds.x + (bounds.width - lineWidth) / 2,
        bounds.y + (bounds.height - font.glyph_height) / 2
    };

    DrawTextScaled(font, textPos.x, textPos.y, text, spacing, 1, textColor);

    if (mouseOver && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        pressed = true;
    }

    return pressed;
}

