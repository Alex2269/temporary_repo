// parse_data.c

#include <string.h> // strstr
#include <stdlib.h> // atoi
#include "parse_data.h"

#include <stdint.h>
#include <stdio.h>

#define PACKET_SIZE 13

// Функція розбору пакета
// packet - масив байтів довжиною PACKET_SIZE
// values - масив з 4 елементів для збереження значень каналів
// Повертає 0 при успіху, -1 при помилці (наприклад, неправильний стартовий байт)
int parse_binary_packet(const uint8_t *packet, uint16_t *values)
{
    if (packet[0] != 0xAA) {
        // Неправильний стартовий байт
        return -1;
    }

    // Ініціалізуємо всі значення -1 (або 0, якщо хочеш)
    for (int i = 0; i < 4; i++) {
        values[i] = 0xFFFF; // наприклад, 0xFFFF - позначка "немає даних"
    }

    // Розбираємо 4 канали
    for (int i = 0; i < 4; i++) {
        uint8_t channel_id = packet[1 + i * 3];
        uint16_t val = packet[1 + i * 3 + 1] | (packet[1 + i * 3 + 2] << 8);

        if (channel_id < 4) {
            values[channel_id] = val;
        } else {
            // Невідомий ID каналу, можна ігнорувати або повертати помилку
            return -1;
        }
    }

    return 0;
}

