// cursor.h

#ifndef CURSOR_H
#define CURSOR_H

#include "raylib.h"
#include <stdbool.h>
#include "psf_font.h"      // Заголовок із парсером PSF-шрифту

// Константи за замовчуванням для курсорів і графіки
#define DEFAULT_CURSOR_WIDTH 10.0f
#define DEFAULT_CURSOR_HEIGHT 20.0f
#define DEFAULT_CURSOR_TOP_Y 5
#define ARROW_SIZE 10
#define THIN_LINE 100

/*
 * Структура для курсора (рухомий маркер по горизонталі)
 */
typedef struct {
    float x;          // Позиція курсора по горизонталі (центр прямокутника)
    float y;          // Позиція для відображення тексту і лінії під курсором (динамічна)
    float topY;       // Верхня позиція курсора по вертикалі (верхній край прямокутника)
    float width;      // Ширина прямокутника курсора
    float height;     // Висота прямокутника курсора
    Color color;      // Колір курсора
    bool isDragging;  // Чи перетягується курсор зараз
    int value;        // Значення курсора (пропорційне позиції по X)
    int minValue;     // Мінімальне значення курсора
    int maxValue;     // Максимальне значення курсора

    int min_X;     // Мінімальна позиція курсора (початкове обмеження)
    int max_X;     // Максимальна позиція курсора (обмеження по X)

} Cursor;

/*
 * Структура для центрального прямокутника (ручки), що розміщується на горизонтальній лінії між курсорами
 */
typedef struct {
    float x;          // Позиція по горизонталі (центр прямокутника)
    float y;          // Позиція по вертикалі (центр прямокутника)
    float width;      // Ширина прямокутника
    float height;     // Висота прямокутника
    Color color;      // Колір прямокутника
    bool isDragging;  // Чи перетягується прямокутник зараз
} DragRect;

/*
 * Ініціалізує курсор з заданими параметрами.
 *
 * Параметри:
 *   startX - початкова позиція по X
 *   topY - верхня позиція по Y (верхній край прямокутника)
 *   width - ширина прямокутника курсора
 *   height - висота прямокутника курсора
 *   color - колір курсора
 *   minValue - мінімальне значення курсора
 *   maxValue - максимальне значення курсора
 *
 * Повертає:
 *   Ініціалізований об'єкт Cursor
 */
Cursor InitCursor(float startX, float topY, float width, float height, Color color, int minValue, int maxValue);

/*
 * Оновлює позиції і обробляє взаємодію з усіма курсорами.
 *
 * Параметри:
 *   cursors - масив курсорів
 *   count - кількість курсорів у масиві
 *   mousePos - поточна позиція миші
 *   mouseButtonPressed - чи було натиснення кнопки миші цього кадру
 *   mouseButtonDown - чи утримується кнопка миші
 *   mouseButtonReleased - чи було відпускання кнопки миші цього кадру
 */
void UpdateAndHandleCursors(Cursor *cursors, int count, Vector2 mousePos, bool mouseButtonPressed, bool mouseButtonDown, bool mouseButtonReleased);

/*
 * Малює курсори, лінії між ними, стрілки, центральний прямокутник та відстань.
 *
 * Параметри:
 *   cursors - масив курсорів
 *   count - кількість курсорів
 *   font - шрифт для тексту
 *   fontSize - розмір шрифту
 *   centerRect - вказівник на центральний прямокутник (ручку)
 */
void DrawCursorsAndDistance(Cursor *cursors, int count, PSF_Font font, int fontSize, DragRect *centerRect);

/*
 * Перевіряє, чи знаходиться курсор миші над прямокутником.
 *
 * Параметри:
 *   mousePos - позиція миші
 *   rect - прямокутник для перевірки
 *
 * Повертає:
 *   true, якщо миша над прямокутником, інакше false
 */
bool IsMouseOverRect(Vector2 mousePos, DragRect rect);

/*
 * Оновлює позицію центрального прямокутника (ручки) при перетягуванні мишею.
 *
 * Параметри:
 *   rect - вказівник на центральний прямокутник
 *   mousePos - позиція миші
 *   mouseButtonPressed - чи було натиснення кнопки миші цього кадру
 *   mouseButtonDown - чи утримується кнопка миші
 *   mouseButtonReleased - чи було відпускання кнопки миші цього кадру
 */
void UpdateAndHandleCenterRect(DragRect *rect, Vector2 mousePos, bool mouseButtonPressed, bool mouseButtonDown, bool mouseButtonReleased);

#endif // CURSOR_H

