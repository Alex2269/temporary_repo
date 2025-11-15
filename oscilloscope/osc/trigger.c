// trigger.c

#include "main.h"
#include "trigger.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * Функція пошуку індексу фронту тригера з урахуванням гістерезису та типу фронту.
 *
 * @param history - масив історії значень сигналу (масштабованих у пікселях)
 * @param history_index - поточний індекс запису у циклічному буфері історії
 * @param trigger_level_px - рівень тригера у пікселях
 * @param trigger_hysteresis_px - гістерезис у пікселях
 * @param history_size - розмір буфера історії
 * @param last_trigger_index - останній зафіксований індекс позиції тригера
 * @param trigger_locked - вказівник на прапорець, що показує, чи тригер захоплений
 * @param trigger_edge - тип фронту тригера (rising, falling, auto)
 *
 * @return індекс позиції спрацьовування тригера, або -1, якщо тригер не спрацював
 */
int find_trigger_index_with_hysteresis(float *history, int history_index,
                                       float trigger_level_px, float trigger_hysteresis_px,
                                       int history_size, int last_trigger_index, bool *trigger_locked,
                                       int trigger_edge) {
    // Якщо тригер вже захоплений, перевіряємо, чи сигнал залишився в межах гістерезису
    if (*trigger_locked) {
        // Обчислюємо індекс у межах циклічного буфера
        int idx = last_trigger_index % history_size;
        float val = history[idx];

        // Перевірка виходу сигналу за межі гістерезису для розблокування тригера
        if (val < trigger_level_px - trigger_hysteresis_px || val > trigger_level_px + trigger_hysteresis_px) {
            *trigger_locked = false; // Розблокуємо тригер для пошуку нового фронту
        } else {
            // Тригер залишається захопленим, повертаємо останній індекс позиції тригера
            return last_trigger_index;
        }
    }

    // Якщо тригер не захоплений, шукаємо новий фронт переходу через рівень тригера з урахуванням гістерезису
    for (int i = 0; i < history_size - 1; i++) {
        // Обчислюємо індекси двох сусідніх точок у циклічному буфері
        int idx0 = (history_index + i) % history_size;
        int idx1 = (history_index + i + 1) % history_size;
        float v0 = history[idx0];
        float v1 = history[idx1];

        // Перевіряємо тип фронту тригера і шукаємо відповідний перехід
        if (trigger_edge == TRIGGER_EDGE_RISING) {
            // Зростаючий фронт: сигнал переходить з нижчого за (trigger_level - гістерезис)
            // до вищого або рівного (trigger_level + гістерезис)
            if (v0 < trigger_level_px - trigger_hysteresis_px && v1 >= trigger_level_px + trigger_hysteresis_px) {
                *trigger_locked = true;
                return idx1;
            }
        } else if (trigger_edge == TRIGGER_EDGE_FALLING) {
            // Спадаючий фронт: сигнал переходить з вищого за (trigger_level + гістерезис)
            // до нижчого або рівного (trigger_level - гістерезис)
            if (v0 > trigger_level_px + trigger_hysteresis_px && v1 <= trigger_level_px - trigger_hysteresis_px) {
                *trigger_locked = true;
                return idx1;
            }
        } else { // TRIGGER_EDGE_AUTO
            // Автоматичний режим: шукаємо будь-який фронт (rising або falling)
            if ((v0 < trigger_level_px - trigger_hysteresis_px && v1 >= trigger_level_px + trigger_hysteresis_px) ||
                (v0 > trigger_level_px + trigger_hysteresis_px && v1 <= trigger_level_px - trigger_hysteresis_px)) {
                *trigger_locked = true;
                return idx1;
            }
        }
    }

    // Якщо фронт не знайдено — тригер не спрацював
    return -1;
}

/**
 * Функція оновлення індексів тригера для всіх каналів осцилографа.
 * Виконує пошук позиції спрацьовування тригера з урахуванням гістерезису, типу фронту та фіксації стану.
 *
 * @param oscData - структура з даними осцилографа та налаштуваннями каналів
 */
void update_trigger_indices(OscData *oscData) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        ChannelSettings *ch = &oscData->channels[i];

        // Перевіряємо, чи канал активний, чи активний тригер і чи є дані історії сигналу
        if (ch->active && ch->trigger_active && ch->channel_history != NULL) {
            // Обчислюємо рівень тригера у пікселях відносно висоти робочої області
            float trigger_level_px = ch->trigger_level * WORKSPACE_HEIGHT;

            // Виконуємо пошук позиції фронту тригера з урахуванням гістерезису та типу фронту
            int new_trigger_index = find_trigger_index_with_hysteresis(
                ch->channel_history,
                oscData->history_index,
                trigger_level_px,
                ch->trigger_hysteresis_px,
                oscData->history_size,
                ch->trigger_index,
                &ch->trigger_locked,
                ch->trigger_edge);  // Передаємо тип фронту

            // Якщо тригер спрацював (знайдено позицію)
            if (new_trigger_index >= 0) {
                // Оновлюємо індекс позиції тригера
                ch->trigger_index = new_trigger_index;
            } else {
                // Якщо тригер не спрацював і не захоплений, можна скинути індекс або залишити попередній
                if (!ch->trigger_locked) {
                    ch->trigger_index = 0; // Значення за замовчуванням (початок буфера)
                }
            }
        } else {
            // Якщо канал не активний або тригер вимкнений — скидаємо стан захоплення і індекс
            ch->trigger_locked = false;
            ch->trigger_index = 0;
        }
    }
}

