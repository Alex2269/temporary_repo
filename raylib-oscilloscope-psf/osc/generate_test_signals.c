// generate_test_signals.c

#include "math.h"
#include <stddef.h>
#include "generate_test_signals.h"

#define PI 3.14159265359f

    // oscData.offset_y_a = screenHeight / 4.0f * 2.5f + 200;
    // oscData.offset_y_b = screenHeight / 4.0f * 3.5f + 200;

        // generate_test_signals(&oscData, 500, 100);
        // Приклад виклику
        // draw_signal(&oscData, osc_width, 2.0f);

        /*
        float current_time = 0.0f;
        generate_test_signals(&oscData, 500, current_time);
        current_time += 0.1f; // Збільшуємо час для наступного кадру */


// void generate_test_signals(OscData *data, int history_size, float time)
// {
//     // Перевірка наявності буферів історії для каналів 0 і 1
//     if (data->channels[2].channel_history == NULL || data->channels[3].channel_history == NULL) return;
//
//     for (int i = 0; i < history_size; i++) {
//         float t = time + i * 0.1f;
//         data->channels[2].channel_history[i] = 200.0f + 100.0f * sinf(t);       // Синусоїда для каналу 0 (A)
//         data->channels[3].channel_history[i] = 200.0f + 100.0f * cosf(t * 0.5f); // Косинусоїда для каналу 1 (B)
//     }
// }

/*
void generate_test_signals(OscData *data, int history_size, float time)
{
    if (data->channels[2].channel_history == NULL || data->channels[3].channel_history == NULL) return;

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.1f;

        // Складний сигнал з гармоніками для каналу 2 (A)
        float fundamental = sinf(t);          // основна частота
        float second_harmonic = 0.5f * sinf(2.0f * t);  // друга гармоніка з меншою амплітудою
        float third_harmonic = 0.3f * sinf(3.0f * t);   // третя гармоніка ще меншої амплітуди

        data->channels[2].channel_history[i] = 200.0f + 100.0f * (fundamental + second_harmonic + third_harmonic);

        // Для каналу 3 (B) залишимо косинусоїду, або теж можна ускладнити аналогічно
        data->channels[3].channel_history[i] = 200.0f + 100.0f * cosf(t * 0.5f);
    }
}*/


void generate_test_signals(OscData *data, int history_size, float time)
{
    if (data->channels[2].channel_history == NULL || data->channels[3].channel_history == NULL) return;

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
        data->channels[2].channel_history[i] = 200.0f + 100.0f * envelope * (fundamental + second_harmonic + third_harmonic);

        // Для каналу 3 залишаємо просту косинусоїду
        data->channels[3].channel_history[i] = 200.0f + 100.0f * cosf(0.5f * t);
    }
}

void generate_third_order_like_signal(OscData *data, int history_size, float time)
{
    if (data->channels[2].channel_history == NULL || data->channels[3].channel_history == NULL) return;

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.1f;

        // Основна гармоніка — великий центральний "горб"
        float fundamental = sinf(t);

        // Друга і третя гармоніки — менші бокові "горби"
        float second_harmonic = 0.6f * sinf(2.0f * t + PI / 4);  // з фазовим зсувом для форми
        float third_harmonic = 0.4f * sinf(3.0f * t + PI / 2);

        // Сума гармонік формує форму сигналу з центральним піком і боковими
        float signal = fundamental + second_harmonic + third_harmonic;

        // Масштабування і зміщення для зручності відображення
        data->channels[2].channel_history[i] = 200.0f + 100.0f * signal;

        // Для каналу 3 залишимо просту косинусоїду
        data->channels[3].channel_history[i] = 200.0f + 100.0f * cosf(0.5f * t);
    }
}

void generate_gaussian_envelope_signal(OscData *data, int history_size, float time)
{
    if (data->channels[2].channel_history == NULL || data->channels[3].channel_history == NULL) return;

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
        data->channels[2].channel_history[i] = 200.0f + 100.0f * signal;

        // Для каналу 3 залишимо просту косинусоїду
        data->channels[3].channel_history[i] = 200.0f + 100.0f * cosf(0.5f * t);
    }
}
