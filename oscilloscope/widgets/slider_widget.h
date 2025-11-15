#ifndef SLIDER_WIDGET_H
#define SLIDER_WIDGET_H

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "color_utils.h"
#include "glyphs.h"

// Функції для роботи зі слайдерами, які тепер керуються централізовано.
// Це дозволяє обробляти накладення та взаємодію курсорів в одному місці.

/**
 * @brief Реєструє новий слайдер або оновлює існуючий.
 *
 * Ця функція лише зберігає параметри слайдера в статичному масиві.
 * Вона не виконує жодної логіки малювання чи взаємодії з мишею.
 *
 * @param sliderIndex Індекс слайдера в масиві (від 0 до MAX_SLIDERS-1).
 * @param bounds Прямокутна область, в якій малюється слайдер.
 * @param value Вказівник на поточне значення слайдера.
 * @param minValue Мінімально допустиме значення.
 * @param maxValue Максимально допустиме значення.
 * @param isVertical Визначає орієнтацію слайдера (вертикальний чи горизонтальний).
 * @param baseColor Основний колір слайдера.
 * @param textTop Текст-підказка над слайдером. Може бути NULL.
 * @param textRight Текст-підказка праворуч від слайдера. Може бути NULL.
 */
void RegisterSlider(int sliderIndex, Rectangle bounds, float *value, float minValue, float maxValue, bool isVertical, Color baseColor, const char* textTop, const char* textRight);

/**
 * @brief Головна функція для оновлення стану та малювання всіх слайдерів.
 *
 * Ця функція централізовано обробляє взаємодію миші з усіма зареєстрованими слайдерами,
 * керує LIFO-порядком малювання та оновлює їх значення. Її слід викликати
 * один раз за кадр у вашому головному циклі малювання.
 *
 * @param font Шрифт для відображення тексту.
 * @param spacing Відстань між символами.
 */
void UpdateSlidersAndDraw(RasterFont font, int spacing);

#endif // SLIDER_WIDGET_H
