// trigger.h
#ifndef TRIGGER_H
#define TRIGGER_H

// Константи для типу фронту тригера
#define TRIGGER_EDGE_RISING 0   // Зростаючий фронт
#define TRIGGER_EDGE_FALLING 1  // Спадаючий фронт
#define TRIGGER_EDGE_AUTO 2     // Автоматичний режим (будь-який фронт)

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
                                       int trigger_edge);

/**
 * Функція оновлення індексів тригера для всіх каналів осцилографа.
 * Виконує пошук позиції спрацьовування тригера з урахуванням гістерезису, типу фронту та фіксації стану.
 *
 * @param oscData - структура з даними осцилографа та налаштуваннями каналів
 */
void update_trigger_indices(OscData *oscData);

#endif // TRIGGER_H

