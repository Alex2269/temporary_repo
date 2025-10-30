// cursor.c

#include "cursor.h"
#include <math.h>
#include <stdio.h>

extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

/*
 * Ініціалізація курсора з початковими параметрами.
 */
Cursor InitCursor(float startX, float topY, float width, float height, Color color, int minValue, int maxValue) {
    Cursor cursor;
    cursor.x = startX;                // Початкова позиція по X
    cursor.topY = topY;               // Верхній край курсора по Y
    cursor.width = width;             // Ширина прямокутника
    cursor.height = height;           // Висота прямокутника
    cursor.color = color;             // Колір курсора
    cursor.isDragging = false;        // Спочатку не перетягується
    cursor.minValue = minValue;       // Мінімальне значення
    cursor.maxValue = maxValue;       // Максимальне значення

    // Обчислення початкового значення курсора пропорційно позиції по X
    cursor.value = minValue + (maxValue - minValue) * ((startX) / GetScreenWidth());

    // Позиція по Y для відображення тексту і лінії (нижче курсора)
    cursor.y = topY + height + THIN_LINE;

    return cursor;
}

/*
 * Перевіряє, чи знаходиться миша над курсором (для початку перетягування).
 */
static bool IsMouseOverCursor(Vector2 mousePos, Cursor cursor) {
    int Sticking = 2; // Розширення області прилипання курсору до миші
    // Область по X розширена в 2 рази від ширини курсора, по Y - висота курсора
    return (mousePos.x > cursor.x - cursor.width * Sticking &&
            mousePos.x < cursor.x + cursor.width * Sticking &&
            mousePos.y > cursor.topY &&
            mousePos.y < cursor.topY + cursor.height);
}

/*
 * Оновлює позицію курсора під час перетягування по горизонталі.
 * Забезпечує уникнення накладання з іншим курсором.
 */
static void UpdateCursorDrag(Cursor *cursor, Vector2 mousePos, Cursor *otherCursor) {
    cursor->x = mousePos.x;  // Оновлюємо позицію по X

    float minX = cursor->min_X + cursor->width / 2;     // Мінімальна позиція по X (щоб не вийти за межі екрану)
    float maxX = cursor->max_X - cursor->width / 2;     // Максимальна позиція по X (обмежуємся osc_width)
    float collisionThreshold = cursor->width * 1.5f;    // Мінімальна відстань між курсорами

    // Перевірка зіткнення з іншим курсором
    if (otherCursor != NULL && cursor->isDragging && cursor != otherCursor) {
        if (cursor->x < otherCursor->x && cursor->x + collisionThreshold > otherCursor->x) {
            cursor->x = otherCursor->x - collisionThreshold;
        }
        else if (cursor->x > otherCursor->x && cursor->x - collisionThreshold < otherCursor->x) {
            cursor->x = otherCursor->x + collisionThreshold;
        }
    }

    // Обмеження позиції курсора в межах екрану
    if (cursor->x < minX) cursor->x = minX;
    if (cursor->x > maxX) cursor->x = maxX;

    // Обчислення значення курсора пропорційно позиції по X
    cursor->value = cursor->minValue + (cursor->maxValue - cursor->minValue) * ((cursor->x - minX) / (maxX - minX));

    // Гарантія, що значення не виходить за межі
    if (cursor->value < cursor->minValue) cursor->value = cursor->minValue;
    if (cursor->value > cursor->maxValue) cursor->value = cursor->maxValue;

    // Оновлення позиції по Y для відображення тексту і лінії
    cursor->y = cursor->topY + cursor->height + THIN_LINE;
}

/*
 * Оновлює всі курсори: обробка натискань, перетягування і відпускання миші.
 */
void UpdateAndHandleCursors(Cursor *cursors, int count, Vector2 mousePos, bool mouseButtonPressed, bool mouseButtonDown, bool mouseButtonReleased) {
    for (int i = 0; i < count; ++i) {
        Cursor *cursor = &cursors[i];
        Cursor *otherCursor = (count > 1) ? &cursors[(i + 1) % count] : NULL;

        // Початок перетягування, якщо миша над курсором і кнопка натиснута
        if (mouseButtonPressed && IsMouseOverCursor(mousePos, *cursor)) {
            cursor->isDragging = true;
        }

        // Оновлення позиції під час перетягування
        if (cursor->isDragging && mouseButtonDown) {
            UpdateCursorDrag(cursor, mousePos, otherCursor);
        }

        // Завершення перетягування при відпусканні кнопки
        if (mouseButtonReleased) {
            cursor->isDragging = false;
        }
    }
}

/*
 * Перевіряє, чи знаходиться миша над прямокутником (ручкою).
 */
bool IsMouseOverRect(Vector2 mousePos, DragRect rect) {
    int Sticking = 2; // Розширення області прилипання курсору до миші
    return (mousePos.x > rect.x - rect.width / 2 &&
            mousePos.x < rect.x + rect.width / 2 &&
            mousePos.y > rect.y - rect.height * Sticking &&
            mousePos.y < rect.y + rect.height * Sticking);
}

/*
 * Оновлює позицію центрального прямокутника (ручки) при перетягуванні по вертикалі.
 * Обмежує рух в межах екрану.
 */
void UpdateAndHandleCenterRect(DragRect *rect, Vector2 mousePos, bool mouseButtonPressed, bool mouseButtonDown, bool mouseButtonReleased) {
    // Початок перетягування, якщо миша над прямокутником і кнопка натиснута
    if (mouseButtonPressed && IsMouseOverRect(mousePos, *rect)) {
        rect->isDragging = true;
    }

    // Оновлення позиції по Y під час перетягування
    if (rect->isDragging && mouseButtonDown) {
        rect->y = mousePos.y;

        // Обмеження вертикального руху в межах екрану (10 пікселів від країв)
        if (rect->y < 10) rect->y = 10;
        if (rect->y > GetScreenHeight() - 10) rect->y = GetScreenHeight() - 10;
    }

    // Завершення перетягування
    if (mouseButtonReleased) {
        rect->isDragging = false;
    }
}

/*
 * Малює курсори, вертикальні лінії під ними, горизонтальну лінію зі стрілками,
 * центральний прямокутник (ручку) та текст відстані між курсорами.
 */
void DrawCursorsAndDistance(Cursor *cursors, int count, RasterFont font, DragRect *centerRect) {

    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    // Оновлення курсорів і центрального прямокутника з урахуванням вводу користувача
    UpdateAndHandleCursors(cursors, count, mousePos, mousePressed, mouseDown, mouseReleased);
    UpdateAndHandleCenterRect(centerRect, mousePos, mousePressed, mouseDown, mouseReleased);

    // Оновлення горизонтальної позиції прямокутника — центр між курсорами
    centerRect->x = (cursors[0].x + cursors[1].x) / 2;

    // Малюємо кожен курсор і вертикальну лінію під ним до горизонтальної лінії
    for (int i = 0; i < count; ++i) {
        Cursor cursor = cursors[i];

        // Малюємо прямокутник курсора
        DrawRectangle(cursor.x - cursor.width / 2, cursor.topY, cursor.width, cursor.height, cursor.color);

        // Обчислюємо довжину вертикальної лінії від нижньої межі курсора до горизонтальної лінії (centerRect->y)
        float lineLength = centerRect->y - (cursor.topY + cursor.height);
        if (lineLength < 0) lineLength = 0; // Запобігаємо малюванню вгору, якщо центр вище курсора

        // Малюємо вертикальну лінію під курсором
        DrawLine(cursor.x, cursor.topY + cursor.height, cursor.x, cursor.topY + cursor.height + lineLength, LIGHTGRAY);
    }

    if (count >= 2) {
        Cursor cursorA = cursors[0];
        Cursor cursorB = cursors[1];
        float distance = fabs(cursorA.x - cursorB.x);
        float lineY = centerRect->y;
        Vector2 middlePoint = {(cursorA.x + cursorB.x) / 2, lineY - 15};

        // Малюємо горизонтальну лінію між курсорами на висоті centerRect->y
        DrawLine(cursorA.x, lineY, cursorB.x, lineY, LIGHTGRAY);

        // Малюємо стрілки на кінцях лінії
        float arrowAOffset = (cursorB.x > cursorA.x) ? ARROW_SIZE : -ARROW_SIZE;
        DrawTriangle(
            (Vector2){cursorA.x, lineY},
            (Vector2){cursorA.x + arrowAOffset, lineY + arrowAOffset / 2},
            (Vector2){cursorA.x + arrowAOffset, lineY - arrowAOffset / 2},
            LIGHTGRAY
        );

        float arrowBOffset = (cursorA.x > cursorB.x) ? ARROW_SIZE : -ARROW_SIZE;
        DrawTriangle(
            (Vector2){cursorB.x, lineY},
            (Vector2){cursorB.x + arrowBOffset, lineY + arrowBOffset / 2},
            (Vector2){cursorB.x + arrowBOffset, lineY - arrowBOffset / 2},
            LIGHTGRAY
        );

        // Малюємо центральний прямокутник (ручку) у центрі лінії
        DrawRectangle(centerRect->x - centerRect->width / 2, centerRect->y - centerRect->height / 2, centerRect->width, centerRect->height, centerRect->color);

        // Формуємо текст відстані між курсорами у пікселях
        char distanceText[32];
        sprintf(distanceText, "%i px", (int)distance);
        int fontSize = font.glyph_height;
        int textWidth = MeasureText(distanceText, fontSize);

        // Малюємо текст відстані у центрі лінії, трохи вище прямокутника
        Vector2 textPos = {middlePoint.x - textWidth / 2, middlePoint.y - fontSize - 5};
        DrawTextScaled(font, textPos.x, textPos.y, distanceText, spacing, 1, LIGHTGRAY);
    }

    // Вивід значень курсорів у правій верхній частині екрану
    Vector2 curPosA = {500, 10};
    DrawTextScaled(font, curPosA.x, curPosA.y, TextFormat("A:%i", cursors[0].value), spacing, 1, cursors[0].color);
    Vector2 curPosB = {550, 10};
    DrawTextScaled(font, curPosB.x, curPosB.y, TextFormat("B:%i", cursors[1].value), spacing, 1, cursors[1].color);
}

