// file read_usb_device.c

#include "read_usb_device.h"
#include "rs232.h"
#include "parse_data.h"

void read_usb_device(OscData *data) {
    static uint8_t buffer[PACKET_SIZE];
    static int buf_idx = 0;

    if (data->comport_number < 0) return;

    uint8_t temp_buf[256];
    int bytes_read = RS232_PollComport(data->comport_number, temp_buf, sizeof(temp_buf));
    if (bytes_read <= 0) return;

    for (int i = 0; i < bytes_read; i++) {
        uint8_t byte = temp_buf[i];

        if (buf_idx == 0) {
            if (byte == 0xAA) buffer[buf_idx++] = byte;
        } else {
            buffer[buf_idx++] = byte;
            if (buf_idx == PACKET_SIZE) {
                // Маємо повний пакет
                int16_t channel_values[4];
                if (parse_binary_packet(buffer, channel_values) == 0) {
                    data->adc_tmp_a = channel_values[0];
                    data->adc_tmp_b = channel_values[1];
                    data->adc_tmp_c = channel_values[2];
                    data->adc_tmp_d = channel_values[3];

                    // Масштабування сигналу до розміру сітки зі зміщенням до центру
                    static int height = 500;
                    float scaled_a = ((float)data->adc_tmp_a / 4095) * height * (data->channels[0].signal_level) - 600/2;
                    float scaled_b = ((float)data->adc_tmp_b / 4095) * height * (data->channels[1].signal_level) - 600/2;
                    float scaled_c = ((float)data->adc_tmp_c / 4095) * height * (data->channels[2].signal_level) - 600/2;
                    float scaled_d = ((float)data->adc_tmp_d / 4095) * height * (data->channels[3].signal_level) - 600/2;

                    if (data->channels[0].channel_history)
                        data->channels[0].channel_history[data->history_index] = scaled_a;
                    if (data->channels[1].channel_history)
                        data->channels[1].channel_history[data->history_index] = scaled_b;
                    if (data->channels[2].channel_history)
                        data->channels[2].channel_history[data->history_index] = scaled_c;
                    if (data->channels[3].channel_history)
                        data->channels[3].channel_history[data->history_index] = scaled_d;

                    // ОНОВЛЕННЯ: використовуємо динамічний розмір буфера!
                    data->history_index = (data->history_index + 1) % data->history_size;
                    if (data->valid_points < data->history_size)
                        data->valid_points++;
                }
                buf_idx = 0;
            }
        }
    }
}

