// file generate_test_signals.h

#ifndef __GENERATE_TEST_SIGNALS_H
#define __GENERATE_TEST_SIGNALS_H

#include "main.h"
#include <stdbool.h>

#define MAX_CHANNELS 4

typedef struct {
    float *channel_history[MAX_CHANNELS]; // Масив вказівників на буфер історії для кожного каналу
} OscData;

// Прототипи функцій малювання тестових сигналів

// Генерація складних тестових сигналів для 4 каналів
void generate_test_signals4(OscData *data, int history_size, float time);

void generate_test_signals(OscData *data, int history_size, float time);
void generate_gaussian_envelope_signal(OscData *data, int history_size, float time);
void generate_test_signals_extended(OscData *data, int history_size, float time);


#endif /* __GENERATE_TEST_SIGNALS_H */

