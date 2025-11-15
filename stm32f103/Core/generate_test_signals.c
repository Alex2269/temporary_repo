#include "math.h"
#include <stddef.h>
#include <stdlib.h>
#include "generate_test_signals.h"

#define PI 3.14159265359f


// Прямокутний сигнал з частотою freq і робочим циклом duty_cycle (0..1)
float square_wave(float t, float freq, float duty_cycle) {
    float period = 1.0f / freq;
    float phase = fmodf(t, period);
    return (phase < duty_cycle * period) ? 1.0f : -1.0f;
}

// Пилкоподібний сигнал з частотою freq
float sawtooth_wave(float t, float freq) {
    float period = 1.0f / freq;
    float phase = fmodf(t, period);
    return (2.0f * (phase / period)) - 1.0f;  // від -1 до 1 лінійно
}

// Імпульсний сигнал з тривалістю pulse_width (у секундах) і частотою freq
float pulse_wave(float t, float freq, float pulse_width) {
    float period = 1.0f / freq;
    float phase = fmodf(t, period);
    return (phase < pulse_width) ? 1.0f : -1.0f;
}

float noise_wave() {
    return 2.0f * ((float)rand() / RAND_MAX) - 1.0f;  // випадкове значення від -1 до 1
}

void generate_test_signals_extended(OscData *data, int history_size, float time)
{
    // Перевірка наявності буферів історії для всіх 4 каналів
    for (int ch = 0; ch < 4; ch++) {
        if (data->channel_history[ch] == NULL) return;
    }

    float freq = 1.0f;       // 1 Гц для прикладу
    float duty_cycle = 0.3f; // 30% робочий цикл
    float pulse_width = 0.05f; // 50 мс імпульс

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.01f;  // крок часу

        // Канал 0: синусоїда з гармоніками (як раніше)
        data->channel_history[0][i] = 250.0f *
        (sinf(t) + 0.5f * sinf(3.0f * t) + 0.3f * sinf(5.0f * t));

        // Канал 1: квадратний сигнал
        data->channel_history[1][i] = 250.0f *
        square_wave(t, freq, duty_cycle);

        // Канал 2: пилкоподібний сигнал
        data->channel_history[2][i] = 250.0f *
        sawtooth_wave(t, freq);

        // Канал 3: імпульсний сигнал з шумом
        float base_pulse = pulse_wave(t, freq, pulse_width);
        float noise = 0.1f * noise_wave();
        data->channel_history[3][i] = 250.0f *
        (base_pulse + noise);
    }
}

