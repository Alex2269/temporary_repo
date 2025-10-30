// file generate_test_signals.h

#ifndef __GENERATE_TEST_SIGNALS_H
#define __GENERATE_TEST_SIGNALS_H

#include "main.h"

// Прототип функції малювання тестових сигналів
void update_test_signals(OscData *data, float *time, float time_step);
void generate_test_signals_extended(OscData *data, int history_size, float time);

#endif /* __GENERATE_TEST_SIGNALS_H */
