#include "gui_radiobutton_row.h"
#include "raylib.h"

extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

// Внутрішня функція кнопки, інтегрована сюди, щоб не залежати від button.c
static bool Gui_Square_Button(Rectangle bounds, RasterFont font, const char *text,
                              Color colorNormal, Color colorHover, Color colorPressed, Color colorText)
{
    Vector2 mousePoint = GetMousePosition();
    bool pressed = false;

    bool mouseOver = CheckCollisionPointRec(mousePoint, bounds);
    Color btnColor = colorNormal;

    if (mouseOver) btnColor = colorHover;
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnColor = colorPressed;

    // Контур кнопки
    Color borderColor = (btnColor.r * 0.299f + btnColor.g * 0.587f + btnColor.b * 0.114f) / 255.0f > 0.5f ? BLACK : WHITE;
    int borderThickness = 2;
    DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
                                    bounds.width + 2 * borderThickness, bounds.height + 2 * borderThickness},
                         borderThickness, borderColor);

    // Малюємо кнопку
    DrawRectangleRec(bounds, btnColor);

    // Колір тексту (якщо альфа 0, підбираємо контрастний)
    Color textColor = (colorText.a == 0) ? borderColor : colorText;

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

int Gui_RadioButtons_Row(Rectangle bounds, RasterFont font, const char **items, int itemCount, int currentIndex, Color colorActive, int buttonSize, int spacing)
{
    int selectedIndex = currentIndex;

    for (int i = 0; i < itemCount; i++)
    {
        Rectangle btnRect = {
            bounds.x + i * (buttonSize + spacing),
            bounds.y,
            (float)buttonSize,
            (float)buttonSize
        };

        // Колір кнопки: активна яскрава, інші - напівпрозорі
        Color btnColor = (i == currentIndex) ? colorActive : Fade(colorActive, 0.25f);

        bool pressed = Gui_Square_Button(btnRect, font, items[i], btnColor, GRAY, DARKGRAY, (Color){0,0,0,0});

        if (pressed)
        {
            selectedIndex = i;
        }
    }

    return selectedIndex;
}

