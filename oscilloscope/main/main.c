// main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep
#include <stdbool.h>
#include <stdint.h>

#include "main.h"
#include "raylib.h"
#include "init_osc_data.h"
#include "setup_channel_buffers.h"
#include "rs232.h"
#include "find_usb_device.h"
#include "read_usb_device.h"
#include "parse_data.h"
#include "draw_grid.h"
#include "DrawVerticalScale.h"
#include "DrawHorizontalScale.h"
#include "trigger.h"
#include "gui_control_panel.h"
#include "draw_signal.h"
#include "generate_test_signals.h"
#include "cursor.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "glyphs.h"

int LineSpacing = 0; // Відступ між рядками тексту

int spacing = 2; // Відступ між символами, той самий, що передається у DrawPSFText
int scale = 1;
int padding = 3;
int borderThickness = 1;


// float current_time = 0.0f;// Ініціалізація часу і кроку оновлення тестового сигналу
// const float time_step = 0.025f; // крок часу для формування нового значення тестового сигналу

int main(void) {
    const int screenWidth = 1000;
    const int screenHeight = 600;

    // Встановлюємо прапорець для мультисемплінгу (покращення якості графіки)
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "Raylib Oscilloscope with Trigger and Scaling");

    SetTargetFPS(60);

    Cursor cursors[2];
    cursors[0] = InitCursor(225.0f, DEFAULT_CURSOR_TOP_Y, DEFAULT_CURSOR_WIDTH, DEFAULT_CURSOR_HEIGHT, RED, 0, 100);
    cursors[1] = InitCursor(425.0f, DEFAULT_CURSOR_TOP_Y, DEFAULT_CURSOR_WIDTH, DEFAULT_CURSOR_HEIGHT, BLUE, 0, 100);

    // Ініціалізація центрального прямокутника (ручки) між курсорами
    DragRect centerRect = {
        .x = (cursors[0].x + cursors[1].x) / 2,  // центр по горизонталі
        .y = cursors[0].y,                        // початкова вертикальна позиція лінії
        .width = 20,                             // ширина прямокутника
        .height = 10,                            // висота прямокутника
        .color = LIGHTGRAY,                       // колір прямокутника
        .isDragging = false                      // спочатку не перетягується
    };

    OscData oscData = {0};
    init_osc_data(&oscData);
    setup_channel_buffers(&oscData);

    find_usb_device(&oscData);

    float frameTime = 0.0f;

    while (!WindowShouldClose()) {
        frameTime += GetFrameTime();

        if (frameTime * 1000.0f >= oscData.refresh_rate_ms) {
            read_usb_device(&oscData);
            update_trigger_indices(&oscData);
            frameTime = 0.0f;
        }

        static bool control_panel_visible = true;
        // Обробка введення для керування панеллю та масштабом
        if (IsKeyPressed(KEY_TAB)) control_panel_visible = !control_panel_visible;

        int panel_width = control_panel_visible ? 350 : 0;
        int osc_width = screenWidth - panel_width;
        int osc_height = screenHeight;

        // Обмеження виходу курсорів за визначені межі
        cursors[0].min_X = cursors[1].min_X = 20;
        cursors[0].max_X = cursors[1].max_X = osc_width - 18;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, osc_width, osc_height, BLACK);
        draw_grid(osc_width, osc_height, 50, 49);

        // Малювання курсорів, ліній, ручки та тексту
        DrawCursorsAndDistance(cursors, 2, Terminus12x6_font, &centerRect);
        DrawTextScaled(Terminus12x6_font, 180, 10, "простий осцилограф на бібліотеці raylib", spacing, scale, GREEN);
        // DrawTextWithAutoInvertedBackground(Terminus12x6_font, 180, 10, "простий осцилограф на бібліотеці raylib", spacing, scale, GREEN, 4,1);

        // Малювання поточних значень каналів в лівому верхнньому куті осцилоскопа
        Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, SKYBLUE };
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (oscData.channels[i].active && oscData.channels[i].channel_history != NULL) {
                float last_value = oscData.channels[i].channel_history[(oscData.history_index + oscData.history_size - 1) % oscData.history_size];
                Vector2 textPos = {82, 10 + i*20};
                DrawTextWithAutoInvertedBackground(Terminus12x6_font, textPos.x, textPos.y,
                                                  TextFormat("Ch%d: %.0f", i+1, last_value),
                                                   spacing, scale,
                                                   channel_colors[i],
                                                   padding, borderThickness);
            }
        }

        ChannelSettings *Ch = &oscData.channels[oscData.active_channel];
        Rectangle scaleArea = { 1, 0, 5, 600};
        // DrawVerticalScale(1, Ch->scale_y, Ch->offset_y, scaleArea, font12, WHITE);
        DrawVerticalScale(1, Ch->signal_level, Ch->offset_y / Ch->signal_level, scaleArea, Terminus12x6_font, WHITE);

        // Виклик горизонтальної шкали у тій самій області (або зміщеній для видимості)
        //  area - прямокутна область (x, y, width, height), де малюється шкала
        Rectangle horScaleArea = { 50, osc_height - 60, osc_width - 100 , 50 };
        DrawHorizontalScale(0, scale, 275 - oscData.trigger_offset_x,
                            horScaleArea, Terminus12x6_font, WHITE);

        // // Малювання горизонтальної лінії тригера (якщо тригер активний)
        // if (Ch->active && Ch->trigger_active) {
        //     float trigger_level_px = Ch->trigger_level * WORKSPACE_HEIGHT;
        //     float y_trigger = /*osc_height / 2 +*/ Ch->offset_y - trigger_level_px * Ch->scale_y;
        //     DrawLine(0, (int)y_trigger, osc_width, (int)y_trigger, channel_colors[oscData.active_channel]);
        // }
        //
        // // Малювання вертикальної лінії тригера (сканування по горизонталі)
        // if (Ch->active && Ch->channel_history != NULL) {
        //     float x_step = (float)osc_width / oscData.history_size;
        //     int rel_index = (oscData.history_size + Ch->trigger_index - oscData.history_index) % oscData.history_size;
        //     float trigger_x_pos = x_step * rel_index;
        //     DrawLine((int)trigger_x_pos, 0, (int)trigger_x_pos, screenHeight, RED);
        // }

        // Генерація і запис тестових сигналів у буфер історії
        // update_test_signals(&oscData, &current_time, time_step);
        // generate_test_signals_extended(&oscData, oscData.history_size, 2.0f);

        draw_signal(&oscData, osc_width, 2.0f);

        // gui_control_panel(&oscData, screenWidth, screenHeight);
        if (control_panel_visible) {
            gui_control_panel(&oscData, screenWidth, screenHeight);
        }

        EndDrawing();
    }

    if (oscData.comport_number != -1) {
        RS232_CloseComport(oscData.comport_number);
        printf("COM порт %d закрито.\n", oscData.comport_number);
    }

    for (int i = 0; i < MAX_CHANNELS; i++) {
        free(oscData.channels[i].channel_history);
        oscData.channels[i].channel_history = NULL;
    }

    // Після виходу з циклу звільняємо пам'ять шрифту

    CloseWindow();

    return 0;
}


