// trigger_control.c

#include "trigger_control.h"
#include "raylib.h"
#include <math.h>

static Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

void trigger_control(OscData *oscData)
{
    static bool dragging_trigger_line = false; // Статус перетягування
    float trigger_y_position = 0.0f;  // Початкове логічне значення
    // Отримати активний канал та його налаштування
    int ch = oscData->active_channel;
    Color activeColor = channel_colors[ch];
    ChannelSettings *Ch = &oscData->channels[ch];

    // Задаємо фіксовані координати лінії тригера по горизонталі (X)
    int x_start = 75;
    int x_end = 100;

    // Колір для лінії та ручки
    Color trigger_color = activeColor;

    // Обчислення поточної вертикальної позиції тригера в пікселях для малювання
    int pixel_y = (int)(Ch->offset_y - (Ch->trigger_level * WORKSPACE_HEIGHT) * Ch->scale_y);

    // Задаємо радіус для логіки захоплення (більший - зона кліку ширша)
    int handle_capture_radius = 12;

    // І радіус для малювання ручки (менший - щоб візуально ручка залишалась компактною)
    int handle_draw_radius = 4;

    // ОБРОБКА ПОЧАТКУ ПЕРЕТЯГУВАННЯ
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        int mouseX = GetMouseX();
        int mouseY = GetMouseY();

        // Перевірка, чи курсор у межах збільшеної зони захоплення навколо ручки (коло з радіусом handle_capture_radius)
        int dx = mouseX - x_start;
        int dy = mouseY - pixel_y;

        if (dx*dx + dy*dy <= handle_capture_radius * handle_capture_radius)
        {
            dragging_trigger_line = true;
        }
    }

    // ОБРОБКА ПЕРЕТЯГУВАННЯ
    if (dragging_trigger_line)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            int mouseY = GetMouseY();

            // Обмеження вертикального руху в межах робочої області
            int top_limit = 5;
            int bottom_limit = 595;

            if (mouseY < top_limit) mouseY = top_limit;
            if (mouseY > bottom_limit) mouseY = bottom_limit;

            // Конвертація піксельної координати у логічну позицію тригера (0..1)
            trigger_y_position = (Ch->offset_y - (float)mouseY) / Ch->scale_y / WORKSPACE_HEIGHT;

            // Оновлення рівня тригера в каналі
            Ch->trigger_level = trigger_y_position;
        }
        else
        {
            // Кінець перетягування при відпусканні кнопки миші
            dragging_trigger_line = false;
        }
    }
    else
    {
        // Синхронізація локальної позиції з поточним значенням рівня тригера
        trigger_y_position = Ch->trigger_level;
    }

    // Оновлення піксельної позиції для малювання після впливу перетягування
    pixel_y = (int)(Ch->offset_y - (trigger_y_position * WORKSPACE_HEIGHT) * Ch->scale_y);

    // МАЛЮВАННЯ ЛІНІЇ ТРИГЕРА (горизонтальна, зафіксована по X)
    DrawLine(x_start-25, pixel_y, x_end, pixel_y, trigger_color);

    // МАЛЮВАННЯ РУЧКИ (маркер) із меншим радіусом для збереження звичного вигляду
    DrawCircle(x_start, pixel_y, handle_draw_radius, trigger_color);
}

