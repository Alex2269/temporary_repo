// guicheckbox.c

#include "guicheckbox.h"
#include <string.h>
#include <stdio.h>

extern int LineSpacing;    // Відступ між рядками тексту
extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

// Прототип функції підрахунку кількості UTF-8 символів у рядку
int utf8_strlen(const char* s);

/**
 * @brief Малює чекбокс із текстом праворуч і підказкою зверху.
 *
 * Функція підтримує багаторядковий текст у підказці (textTop) і тексті праворуч (textRight).
 * Підказка відображається при наведенні миші на чекбокс.
 *
 * @param bounds Прямокутник чекбокса (позиція і розмір квадрата)
 * @param checked Вказівник на стан чекбокса (true - встановлено, false - ні)
 * @param textTop Текст підказки, що відображається зверху при наведенні (може бути багаторядковим)
 * @param textRight Текст, що відображається праворуч від чекбокса (підтримує перенос рядків)
 * @param color Колір чекбокса, коли він активний
 */
void Gui_CheckBox(Rectangle bounds, bool *checked, RasterFont font, const char *textTop, const char *textRight, Color color)
{
    Vector2 mousePoint = GetMousePosition();               // Поточна позиція миші
    bool mouseOver = CheckCollisionPointRec(mousePoint, bounds);  // Чи наведена миша на чекбокс

    // Визначаємо колір квадрата чекбокса залежно від стану (встановлено/не встановлено) і наведення миші
    Color boxColor = (*checked) ? color : LIGHTGRAY;
    if (mouseOver) boxColor = Fade(boxColor, 0.8f);        // Злегка прозорий при наведенні

    // Малюємо квадрат чекбокса
    DrawRectangleRec(bounds, boxColor);

    // Колір рамки – контрастний до фону чекбокса
    Color borderColor = GetContrastColor(boxColor);
    DrawRectangleLinesEx((Rectangle){bounds.x - 2, bounds.y - 2, bounds.width + 4, bounds.height + 4}, 2, borderColor);

    // Малюємо галочку, якщо чекбокс активний
    if (*checked)
    {
        Vector2 p1 = {bounds.x + bounds.width * 0.2f, bounds.y + bounds.height * 0.5f};
        Vector2 p2 = {bounds.x + bounds.width * 0.45f, bounds.y + bounds.height * 0.75f};
        Vector2 p3 = {bounds.x + bounds.width * 0.8f, bounds.y + bounds.height * 0.25f};
        DrawLineEx(p1, p2, 3, borderColor);
        DrawLineEx(p2, p3, 3, borderColor);
    }

    Color textColor = GetContrastColor(boxColor);  // Колір тексту для читабельності
    int localSpacing = 2;  // Відступ між символами (локальна змінна для уникнення конфліктів)
    int padding = 4;       // Відступи навколо тексту (padding)

    // --- Малюємо текст праворуч (textRight) з підтримкою багаторядковості ---

    if(textRight != NULL && textRight[0] != '\0') {

        // Підрахунок кількості рядків і максимальної ширини рядка для textRight
        int lineCountRight = 1;
        int maxLineWidthCharsRight = 0;
        int currentLineWidthCharsRight = 0;
        for (const char* p = textRight; *p != '\0'; p++) {
            if (*p == '\n') {
                lineCountRight++;
                if (currentLineWidthCharsRight > maxLineWidthCharsRight) maxLineWidthCharsRight = currentLineWidthCharsRight;
                currentLineWidthCharsRight = 0;
            } else {
                currentLineWidthCharsRight++;
            }
        }
        if (currentLineWidthCharsRight > maxLineWidthCharsRight) maxLineWidthCharsRight = currentLineWidthCharsRight;

        // Обчислення ширини і висоти блоку тексту праворуч з урахуванням міжсимвольного відступу і padding
        float textRightWidth = maxLineWidthCharsRight * (font.glyph_width + localSpacing) - localSpacing + 2 * padding;
        float textRightHeight = lineCountRight * font.glyph_height + (lineCountRight - 1) * LineSpacing + 2 * padding;

        // Позиція тексту праворуч, вертикально центрована відносно чекбокса
        Vector2 textRightPos = {bounds.x + bounds.width + 10 + padding, bounds.y + (bounds.height - textRightHeight) / 2 + padding / 2};

        // Прямокутник фону під текстом праворуч
        Rectangle textRightBg = {
            textRightPos.x - padding,
            textRightPos.y - padding / 2,
            textRightWidth,
            textRightHeight
        };

        // Малюємо фон і рамку для тексту праворуч
        DrawRectangleRec(textRightBg, boxColor);
        DrawRectangleLinesEx(textRightBg, 1, borderColor);

        // Малюємо текст праворуч з підтримкою переносу рядків
        DrawTextScaled(font, textRightPos.x, textRightPos.y, textRight, localSpacing, 1, textColor);
    }

    // --- Малюємо підказку зверху (textTop) при наведенні миші ---

    if (mouseOver && textTop && textTop[0] != '\0')
    {
        // Розбиття тексту підказки на рядки (максимум 10 рядків)
        const char* lines[10];
        int lineCountTop = 0;

        char tempText[256];  // Тимчасовий буфер для копії тексту підказки (щоб не змінювати оригінал)
        strncpy(tempText, textTop, sizeof(tempText) - 1);
        tempText[sizeof(tempText) - 1] = '\0';

        char* line = strtok(tempText, "\n");
        while (line != NULL && lineCountTop < 10) {
            lines[lineCountTop++] = line;
            line = strtok(NULL, "\n");
        }

        // Обчислення максимальної ширини рядка з урахуванням кількості символів UTF-8
        float maxWidthTop = 0;
        for (int i = 0; i < lineCountTop; i++) {
            int charCount = utf8_strlen(lines[i]);  // Кількість символів у рядку UTF-8
            float lineWidth = charCount * (font.glyph_width + localSpacing) - localSpacing;
            if (lineWidth > maxWidthTop) maxWidthTop = lineWidth;
        }

        float lineHeightTop = (float)font.glyph_height;
        // Загальна висота підказки з урахуванням міжрядкових відступів і padding
        float totalHeightTop = lineCountTop * lineHeightTop + (lineCountTop - 1) * 2 + 2 * padding;

        // Прямокутник фону підказки зверху, центрований по горизонталі над чекбоксом
        Rectangle tooltipRect = {
            bounds.x + bounds.width / 2.0f - (maxWidthTop + 2 * padding) / 2.0f,
            bounds.y - totalHeightTop - 8,  // Відступ зверху 8 пікселів
            maxWidthTop + 2 * padding,
            totalHeightTop
        };

        // Малюємо фон підказки з напівпрозорим чорним кольором
        DrawRectangleRec(tooltipRect, Fade(BLACK, 0.8f));
        // Малюємо рамку підказки білим кольором
        DrawRectangleLinesEx(tooltipRect, 1, WHITE);

        // Малюємо кожен рядок тексту підказки з вертикальним інтервалом 2 пікселі
        for (int i = 0; i < lineCountTop; i++) {
            DrawTextScaled(font,
                        tooltipRect.x + padding,
                        tooltipRect.y + padding / 2 + i * (lineHeightTop + 2),
                        lines[i], localSpacing, 1, WHITE);
        }
    }

    // --- Обробка кліку миші ---

    // Якщо миша наведена на чекбокс і натиснута ліва кнопка — перемикаємо стан чекбокса
    if (mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        *checked = !(*checked);
    }
}
