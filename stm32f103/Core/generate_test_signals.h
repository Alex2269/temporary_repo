// file generate_test_signals.h

#ifndef __GENERATE_TEST_SIGNALS_H
#define __GENERATE_TEST_SIGNALS_H

#include "main.h"
#include <stdbool.h>

#define MAX_CHANNELS 4

typedef struct {
    float *channel_history[MAX_CHANNELS]; // Масив вказівників на буфер історії для кожного каналу
} OscData;

void generate_test_signals_extended(OscData *data, int history_size, float time);

#endif /* __GENERATE_TEST_SIGNALS_H */

