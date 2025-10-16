// file draw_signal.c

#include "draw_signal.h"

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include "main.h" // Для OscData, ChannelSettings

#define MAX_CHANNELS 4

void draw_signal(OscData *oscData, float osc_width, float lineThickness)
{
    Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

    // Визначаємо, скільки точок реально можна малювати
    int pts = oscData->points_to_display;
    if (pts > oscData->valid_points) pts = oscData->valid_points;
    if (pts > oscData->history_size) pts = oscData->history_size;
    if (pts < 2) return; // нічого малювати

    for (int i = 0; i < MAX_CHANNELS; i++) {
        ChannelSettings *ch = &oscData->channels[i];
        if (!ch->active || ch->channel_history == NULL) continue;

        int history_index = oscData->history_index;
        if (!oscData->movement_signal) history_index = 0;

        // Горизонтальний розподіл навколо тригера
        int points_left, points_right;
        float trigger_x_pos;

        if (!oscData->reverse_signal) {
            points_left = (int)(oscData->trigger_offset_x / osc_width * pts);
            if (points_left < 1) points_left = 1;
            if (points_left > pts - 1) points_left = pts - 1;
            points_right = pts - points_left;
            trigger_x_pos = oscData->trigger_offset_x;
        } else {
            points_right = (int)(oscData->trigger_offset_x / osc_width * pts);
            if (points_right < 1) points_right = 1;
            if (points_right > pts - 1) points_right = pts - 1;
            points_left = pts - points_right;
            trigger_x_pos = osc_width - oscData->trigger_offset_x;
        }

        // Downsampling: крок для зменшення кількості ліній
        int total_points = points_left + points_right;
        if (total_points > pts) total_points = pts;
        int step = (total_points > pts) ? (total_points / pts) : 1;
        if (total_points / step < 2) step = 1;

        float x_step = osc_width / (float)(total_points - 1);

        // Малюємо сигнал
        if (!oscData->reverse_signal) {
            // Ліва частина (до тригера)
            for (int j = 0; j < points_left - step && j + step < oscData->valid_points; j += step) {
                int idx1 = (history_index + ch->trigger_index - points_left + j + oscData->history_size) % oscData->history_size;
                int idx2 = (history_index + ch->trigger_index - points_left + j + step + oscData->history_size) % oscData->history_size;
                Vector2 p1 = { trigger_x_pos - (points_left - 1 - j) * x_step, ch->offset_y - ch->channel_history[idx1] * ch->scale_y };
                Vector2 p2 = { trigger_x_pos - (points_left - 1 - (j + step)) * x_step, ch->offset_y - ch->channel_history[idx2] * ch->scale_y };
                DrawLineEx(p1, p2, lineThickness, channel_colors[i]);
            }
            // Права частина (після тригера)
            for (int j = 0; j < points_right - step && points_left + j + step < oscData->valid_points; j += step) {
                int idx1 = (history_index + ch->trigger_index + j) % oscData->history_size;
                int idx2 = (history_index + ch->trigger_index + j + step) % oscData->history_size;
                Vector2 p1 = { trigger_x_pos + j * x_step, ch->offset_y - ch->channel_history[idx1] * ch->scale_y };
                Vector2 p2 = { trigger_x_pos + (j + step) * x_step, ch->offset_y - ch->channel_history[idx2] * ch->scale_y };
                DrawLineEx(p1, p2, lineThickness, channel_colors[i]);
            }
        } else {
            // Реверс: малюємо справа наліво
            for (int j = 0; j < points_right - step && j + step < oscData->valid_points; j += step) {
                int idx1 = (history_index + ch->trigger_index + points_right - 1 - j) % oscData->history_size;
                int idx2 = (history_index + ch->trigger_index + points_right - 1 - (j + step)) % oscData->history_size;
                Vector2 p1 = { trigger_x_pos + j * x_step, ch->offset_y - ch->channel_history[idx1] * ch->scale_y };
                Vector2 p2 = { trigger_x_pos + (j + step) * x_step, ch->offset_y - ch->channel_history[idx2] * ch->scale_y };
                DrawLineEx(p1, p2, lineThickness, channel_colors[i]);
            }
            for (int j = 0; j < points_left - step && points_right + j + step < oscData->valid_points; j += step) {
                int idx1 = (history_index + ch->trigger_index - points_left + j + oscData->history_size) % oscData->history_size;
                int idx2 = (history_index + ch->trigger_index - points_left + j + step + oscData->history_size) % oscData->history_size;
                Vector2 p1 = { trigger_x_pos - j * x_step, ch->offset_y - ch->channel_history[idx1] * ch->scale_y };
                Vector2 p2 = { trigger_x_pos - (j + step) * x_step, ch->offset_y - ch->channel_history[idx2] * ch->scale_y };
                DrawLineEx(p1, p2, lineThickness, channel_colors[i]);
            }
        }
    }
}

