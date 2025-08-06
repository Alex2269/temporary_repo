#include "math.h"
#include <stddef.h>
#include <stdlib.h>
#include "generate_test_signals.h"

#define PI 3.14159265359f

// Генерація складних тестових сигналів для 4 каналів
void generate_test_signals4(OscData *data, int history_size, float time)
{
    // Перевірка наявності буферів історії для всіх 4 каналів
    for (int ch = 0; ch < 4; ch++) {
        if (data->channel_history[ch] == NULL) return;
    }

    float decay_rate = 0.05f; // швидкість експоненціального затухання

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.1f;
        float envelope = expf(-decay_rate * t);

        // Канал 0: суміш синусоїд з різними частотами і амплітудами
        data->channel_history[0][i] = 1000.0f + 400.0f * envelope * (
            sinf(t) + 0.5f * sinf(3.0f * t) + 0.3f * sinf(5.0f * t));

        // Канал 1: складний сигнал з косинусоїдою і експоненціальним затуханням
        data->channel_history[1][i] = 1000.0f + 350.0f * envelope * (
            cosf(0.8f * t) + 0.4f * cosf(2.5f * t));

        // Канал 2: сигнал з гармоніками і експоненціальним затуханням
        data->channel_history[2][i] = 1000.0f + 500.0f * envelope * (
            sinf(t) + 0.5f * sinf(2.0f * t) + 0.3f * sinf(3.0f * t));

        // Канал 3: простіша косинусоїда з повільною частотою
        data->channel_history[3][i] = 1000.0f + 450.0f * cosf(0.5f * t);
    }
}

void generate_test_signals(OscData *data, int history_size, float time)
{
    if (data->channel_history[2] == NULL || data->channel_history[3] == NULL) return;

    float decay_rate = 0.1f; // швидкість затухання

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.1f;

        // Експоненціальне затухання амплітуди
        float envelope = expf(-decay_rate * t);

        // Складний сигнал з гармоніками для каналу 2 (A)
        // Гармоніки з різними амплітудами
        float fundamental = sinf(t);          // основна частота
        float second_harmonic = 0.5f * sinf(2.0f * t);  // друга гармоніка з меншою амплітудою
        float third_harmonic = 0.3f * sinf(3.0f * t);   // третя гармоніка ще меншої амплітуди

        // Складний затухаючий сигнал для каналу 2
        data->channel_history[2][i] = 1000.0f + 500.0f * envelope * (fundamental + second_harmonic + third_harmonic);

        // Для каналу 3 залишаємо просту косинусоїду
        data->channel_history[3][i] = 1000.0f + 500.0f * cosf(0.5f * t);
    }
}

void generate_gaussian_envelope_signal(OscData *data, int history_size, float time)
{
    if (data->channel_history[2] == NULL || data->channel_history[3] == NULL) return;

    float center_time = 10.0f;  // час центру гауссової функції
    float sigma = 4.0f;        // ширина гауссової функції
    float fundamental_freq = 1.0f; // основна частота сигналу

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.1f;

        // Гауссова огинаюча
        float gaussian_envelope = expf(-powf(t - center_time, 2) / (2 * sigma * sigma));

        // Синусоїда, модульована гауссовою огинаючою
        float signal = gaussian_envelope * sinf(2 * PI * fundamental_freq * t);

        // Масштабування і зміщення для зручного відображення
        data->channel_history[2][i] = 1000.0f + 500.0f * signal;

        // Для каналу 3 залишимо просту косинусоїду
        data->channel_history[3][i] = 1000.0f + 500.0f * cosf(0.5f * t);
    }
}

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
    return (phase < pulse_width) ? 1.0f : 0.0f;
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
        data->channel_history[0][i] = 1000.0f + 400.0f * (
            sinf(t) + 0.5f * sinf(3.0f * t) + 0.3f * sinf(5.0f * t));

        // Канал 1: квадратний сигнал
        data->channel_history[1][i] = 1000.0f + 300.0f * square_wave(t, freq, duty_cycle);

        // Канал 2: пилкоподібний сигнал
        data->channel_history[2][i] = 1000.0f + 350.0f * sawtooth_wave(t, freq);

        // Канал 3: імпульсний сигнал з шумом
        float base_pulse = pulse_wave(t, freq, pulse_width);
        float noise = 0.1f * noise_wave();
        data->channel_history[3][i] = 1000.0f + 400.0f * (base_pulse + noise);
    }
}

