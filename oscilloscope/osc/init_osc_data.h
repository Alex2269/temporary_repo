// file init_osc_data.h

#ifndef INIT_OSC_DATA_H
#define INIT_OSC_DATA_H

#include "main.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_CHANNELS 4
#define PACKET_SIZE 13

typedef struct {
    bool active;
    float scale_y;               // Масштабування по вертикалі (розтягування по вертикалі)
    float signal_level;          // Налаштування рівня сигналу
    float offset_y;              // Зміщення по вертикалі
    float trigger_level;         // Рівень тригера для (0..1)
    float trigger_hysteresis_px; // Визначаємо гістерезис у пікселях
    bool trigger_active;         // Чи активний тригер
    float *channel_history;      // Вказівник на буфер історії (масив float довжиною 500)

    int trigger_edge;            // Шукає фронт відповідно до типу
    int trigger_index;           // Індекс точки тригера в історії
    float trigger_index_smooth;  // Плавне значення індексу тригера (float)
    bool trigger_locked;         // Прапорець блокування оновлення тригера
    int frames_since_trigger;    // Лічильник кадрів після спрацювання тригера
} ChannelSettings;


// Структура для зберігання стану осцилографа і параметрів відображення
typedef struct OscData {
    ChannelSettings channels[MAX_CHANNELS];
    int active_channel;           // індекс активного каналу
    int comport_number;           // Індекс відкритого COM-порту (-1 якщо не відкрито)
    int ray_speed;                // Затримка читання даних у мікросекундах
    int adc_tmp_a;                // Поточне відфільтроване значення ADC каналу A
    int adc_tmp_b;                // Поточне відфільтроване значення ADC каналу B
    int adc_tmp_c;                // Поточне відфільтроване значення ADC каналу A
    int adc_tmp_d;                // Поточне відфільтроване значення ADC каналу B
    int history_index;            // Поточний індекс запису в історії (циклічний буфер)
    float refresh_rate_ms;        // Частота оновлення інтерфейсу (мс)
    bool auto_connect;            // Прапорець автоматичного підключення до COM-порту
    char com_port_name_input[20]; // Ім'я COM-порту, введене користувачем
    bool com_port_name_edit_mode; // Режим редагування імені COM-порту

    // Поля для плавного оновлення тригера і блокування оновлення
    float trigger_offset_x;       // Горизонтальне зміщення позиції тригера (пікселі)
    bool reverse_signal;          // Прапорець реверсу напрямку малювання сигналу
    bool movement_signal;         // Прапорець прокручування малювання сигналу
    bool test_signal;             // Прапорець для запросу малювання тестового сигналу

    bool dynamic_buffer_mode;     // true = динамічний буфер, false = звичайний
    int history_size;             // поточний розмір буфера
    int valid_points;             // Кількість реально отриманих точок
    int points_to_display;        // Початкове число точок для відображення
} OscData;

void init_osc_data(OscData *oscData);

#endif // INIT_OSC_DATA_H

