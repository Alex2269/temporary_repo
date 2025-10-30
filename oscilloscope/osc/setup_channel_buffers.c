// file setup_channel_buffers.c

#include "setup_channel_buffers.h"
#include <stdio.h>
#include <stdlib.h>

void setup_channel_buffers(OscData *oscData) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (oscData->channels[i].channel_history) free(oscData->channels[i].channel_history);
        oscData->channels[i].channel_history = (float*)calloc(oscData->points_to_display, sizeof(float));

        if (!oscData->channels[i].channel_history) {
            fprintf(stderr, "Memory allocation failed for channel %d\n", i);
            // Можна додати аварійне завершення або очищення
        }
    }

    oscData->history_size = oscData->points_to_display;
    oscData->valid_points = 0;
    oscData->history_index = 0;
}

