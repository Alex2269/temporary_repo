// file generate_test_signals.h

#ifndef __GENERATE_TEST_SIGNALS_H
#define __GENERATE_TEST_SIGNALS_H

#include "main.h"

// Прототип функції малювання тестових сигналів
void generate_test_signals(OscData *data, int history_size, float time);
void generate_third_order_like_signal(OscData *data, int history_size, float time);
void generate_gaussian_envelope_signal(OscData *data, int history_size, float time);

#endif /* __GENERATE_TEST_SIGNALS_H */
