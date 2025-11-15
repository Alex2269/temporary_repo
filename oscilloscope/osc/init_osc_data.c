// file init_osc_data.c

#include "init_osc_data.h"

void init_osc_data(OscData *oscData) {
    oscData->comport_number = -1;
    oscData->active_channel = 0;
    oscData->refresh_rate_ms = 20.0f;
    oscData->auto_connect = false;
    oscData->com_port_name_edit_mode = false;
    strcpy(oscData->com_port_name_input, "COM1");
    oscData->ray_speed = 1000;

    for (int i = 0; i < MAX_CHANNELS; i++) {
        oscData->channels[i].scale_y = 1.0f;
        oscData->channels[i].signal_level = 1.0f;
        oscData->channels[i].offset_y = 0;
        oscData->channels[i].trigger_level = -0.50f;
        oscData->channels[i].trigger_hysteresis_px = 0.25f;
        oscData->channels[i].trigger_active = false;
        oscData->channels[i].active = (i < 4);
        oscData->channels[i].channel_history = NULL;
        oscData->channels[i].trigger_edge = 0;
        oscData->channels[i].trigger_index = 0;
        oscData->channels[i].trigger_index_smooth = 0.0f;
        oscData->channels[i].trigger_locked = false;
        oscData->channels[i].frames_since_trigger = 0;
    }

    oscData->history_index = 0;
    oscData->trigger_offset_x = 100;
    oscData->reverse_signal = false;
    oscData->movement_signal = false;
    oscData->test_signal = true;
    oscData->valid_points = 0;
    oscData->points_to_display = 500; // Початкове число точок для відображення
    oscData->dynamic_buffer_mode = true;
    // channel_history виділяється через setup_channel_buffers!
}

