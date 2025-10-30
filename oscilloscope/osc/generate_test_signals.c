// generate_test_signals.c

#include "math.h"
#include <stddef.h>
#include "generate_test_signals.h"

#define PI 3.14159265359f


#include <stdint.h>
#include "math.h"
#include <stddef.h>
#include <stdlib.h>

#define PACKET_SIZE 13
#define MAX_CHANNELS 4

// Структури з вашим визначенням OscData і channels мають містити channel_history як float*

// typedef struct {
//     float *channel_history;
// } Channel;
//
// typedef struct {
//     Channel channels[4];
// } OscData;

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
        if (data->channels[ch].channel_history == NULL) return;
    }

    float freq = 1.0f;       // 1 Гц для прикладу
    float duty_cycle = 0.3f; // 30% робочий цикл
    float pulse_width = 0.05f; // 50 мс імпульс

    float val = 0.0f;

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.01f;  // крок часу

        // Канал 0: синусоїда з гармоніками (як раніше)
        val = 1000.0f + 400.0f * (sinf(t) + 0.5f * sinf(3.0f * t) + 0.3f * sinf(5.0f * t));
        data->channels[0].channel_history[i] = val / 4095 * WORKSPACE_HEIGHT;

        // Канал 1: квадратний сигнал
        val = 1000.0f + 300.0f * square_wave(t, freq, duty_cycle);
        data->channels[1].channel_history[i] = val / 4095 * WORKSPACE_HEIGHT;

        // Канал 2: пилкоподібний сигнал
        val = 1000.0f + 350.0f * sawtooth_wave(t, freq);
        data->channels[2].channel_history[i] = val / 4095 * WORKSPACE_HEIGHT;

        // Канал 3: імпульсний сигнал з шумом
        float base_pulse = pulse_wave(t, freq, pulse_width);
        float noise = 0.1f * noise_wave();
        val = 1000.0f + 400.0f * (base_pulse + noise);
        data->channels[3].channel_history[i] = val / 4095 * WORKSPACE_HEIGHT;
    }
}

void update_test_signals(OscData *data, float *time, float time_step)
{
    if (!data) return;

    int idx = data->history_index;

    float freq = 1.0f;
    float duty_cycle = 0.3f;
    float pulse_width = 0.05f;
    // float WORKSPACE_HEIGHT = 550.0f;

    float t = *time;
    float val = 0.0f;

    for (int ch = 0; ch < MAX_CHANNELS; ch++)
    {
        switch (ch)
        {
            case 0:
                val = 1000.0f + 400.0f * (sinf(t) + 0.5f * sinf(3.0f * t) + 0.3f * sinf(5.0f * t));
                break;
            case 1:
                val = 1000.0f + 300.0f * square_wave(t, freq, duty_cycle);
                break;
            case 2:
                val = 1000.0f + 350.0f * sawtooth_wave(t, freq);
                break;
            case 3:
            {
                float base_pulse = pulse_wave(t, freq, pulse_width);
                float noise = 0.1f * noise_wave();
                val = 1000.0f + 400.0f * (base_pulse + noise);
            }
            break;
            default:
                val = 0.0f;
                break;
        }

        val = val / 4095.0f * WORKSPACE_HEIGHT;

        if (val < 0) val = 0;
        if (val > 4095) val = 4095;

        data->channels[ch].channel_history[idx] = val;
    }

    data->history_index = (idx + 1) % data->history_size;
    *time += time_step;
}
