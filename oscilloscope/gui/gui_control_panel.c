// gui_control_panel.c

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"
#include "main.h" // для OscData, MAX_CHANNELS
#include "gui_control_panel.h"
#include "button.h"
#include "guicheckbox.h"
#include "gui_slider_spinner.h"
#include "slider_widget_circle.h"
#include "slider_widget.h"
#include "cam_switch.h"
#include "knob_gui.h"
#include "setup_channel_buffers.h"
#include "rs232.h"
// #include "gui_radiobutton.h"
#include "gui_radiobutton_row.h"
#include "trigger_control.h"
#include "color_utils.h"

extern int LineSpacing;    // Відступ між рядками тексту
extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

// Статичний масив для стану відкриття radiobutton для кожного каналу
static bool radiobuttonOpen[MAX_CHANNELS] = { false };
// static const char *radiobuttonItems[] = { "Rising", "Falling", "Auto" };
static const char *radiobuttonItems[] = { "R", "F", "A" };

void send_command(OscData *data, char* command, size_t buffer_size, int number);
// void write_usb_device(OscData *data, unsigned char* str);
void write_usb_device(OscData *data, unsigned char* str, size_t len);

// Функція відображає панель керування параметрами осцилографа
// Масив кольорів каналів
static Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

void gui_control_panel(OscData *oscData, int screenWidth, int screenHeight) {
    // Позиції і розміри панелі
    int panelX = screenWidth - 345;
    int panelY = 10;
    int panelWidth = 340;
    int panelHeight = screenHeight - 20;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(DARKGRAY, 0.9f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GRAY);

    // Заголовок панелі
    // DrawText("Control Panel", panelX + 10, panelY + 10, 20, WHITE);
    // DrawPSFText(font12, panelX + 20, panelY + 10, "Панель Керування", 1, WHITE);

    // Кнопки вибору активного каналу з кольорами
    for (int i = 0; i < MAX_CHANNELS; i++) {
        Rectangle btnRect = { panelX + 20 + i * 80, panelY + 20, 60, 30 };
        Color btnColor = (oscData->active_channel == i) ? channel_colors[i] : Fade(channel_colors[i], 0.5f);

        if (Gui_Button(btnRect, TerminusBold18x10_font, TextFormat("CH%d", i + 1), btnColor, GRAY, DARKGRAY, (Color){0,0,0,0})) {
            oscData->active_channel = i;
        }
    }

    // розміри слайдерів
    int W_size = 200;
    int H_size = 30;
    // Відступи для слайдерів
    int sliderX = panelX + 20;
    int sliderY = panelY + 90;
    int spacingY = 140;

    int ch = oscData->active_channel;
    Color activeColor = channel_colors[ch];
    ChannelSettings *Ch = &oscData->channels[ch];

    sliderY += 40;

    int knob_radius = 45;
    // Масштабування по вертикалі
    int Cam0 = Gui_Knob_Channel(0, Terminus12x6_font, TerminusBold18x10_font,
                                sliderX + 65, sliderY,
                                TextFormat("Масштабування CH%d\nпо вертикалі", (int)oscData->active_channel + 1),
                                NULL/*TextFormat("%0.1f", Ch->scale_y)*/,
                                knob_radius,
                                &Ch->scale_y, 0.2f, 2.20f, true, activeColor);
    // Додавання цього рядку перетворює регулятор на дискретний перемикач
    if(Cam0) Ch->scale_y = roundf(Ch->scale_y / 0.02f) * 0.02f; // кратність 0.02f

    // Зміщення по вертикалі
    int Cam1 = Gui_Knob_Channel(1, Terminus12x6_font, TerminusBold18x10_font,
                                sliderX + 234, sliderY,
                                "Vertical\noffset",
                                NULL/*TextFormat("%d", (int)Ch->offset_y)*/,
                                knob_radius,
                                &Ch->offset_y, 250.0f, -250.0f, true, activeColor);
    // Перетворюємо регулятор на дискретний перемикач
    if(Cam1) Ch->offset_y = roundf(Ch->offset_y / 5) * 5; // кратність 5 одиниць

    sliderY += spacingY;

    // Рівень тригера
    int Cam2 = Gui_Knob_Channel(2, Terminus12x6_font, TerminusBold18x10_font,
                                sliderX + 65, sliderY,
                                "Trigger\nlevel",
                                NULL/*TextFormat("%0.1f", Ch->trigger_level)*/,
                                knob_radius,
                                &Ch->trigger_level, -1.0f, 1.0f, true, activeColor);
    // Додавання цього рядку перетворює регулятор на дискретний перемикач
    if(Cam2) Ch->trigger_level = roundf(Ch->trigger_level / 0.02f) * 0.02f; // кратність 0.02f

    trigger_control(oscData);

    int Cam3 = Gui_Knob_Channel(3, Terminus12x6_font, TerminusBold18x10_font,
                                sliderX + 234, sliderY,
                                "Voltage\nvolume",
                                NULL/*TextFormat("%0.1f", Ch->signal_level)*/,
                                knob_radius,
                                &Ch->signal_level, 0.2f, 2.2f, true, activeColor);
    // Додавання цього рядку перетворює регулятор на дискретний перемикач
    if(Cam3) Ch->signal_level = roundf(Ch->signal_level / 0.02f) * 0.02f; // кратність 0.02f

    // Рівень гістерезису
    // Gui_Knob_Channel(4, Terminus12x6_font, TerminusBold18x10_font,
    //                  sliderX + 234, sliderY,
    //                  "Trigger\nhysteresis",
    //                  NULL/*TextFormat("%0.1f", Ch->trigger_hysteresis_px)*/,
    //                  knob_radius,
    //                  &Ch->trigger_hysteresis_px, -1.0f, 1.0f, true, activeColor);

    // Ch->trigger_hysteresis_px = 0.25f;

    // Малювання горизонтальної лінії тригера (якщо тригер активний)
    if (Ch->active && Ch->trigger_active) {
        float trigger_level_px = Ch->trigger_level * WORKSPACE_HEIGHT;
        float y_trigger = /*osc_height / 2 +*/ Ch->offset_y - trigger_level_px * Ch->scale_y;
        DrawLine(0, (int)y_trigger, panelX, (int)y_trigger, activeColor);
    }

    // Малювання вертикальної лінії тригера (сканування по горизонталі)
    if (Ch->active && Ch->channel_history != NULL) {
        float x_step = (float)panelX / oscData->history_size;
        int rel_index = (oscData->history_size + Ch->trigger_index - oscData->history_index) % oscData->history_size;
        float trigger_x_pos = x_step * rel_index;
        DrawLine((int)trigger_x_pos, 0, (int)trigger_x_pos, screenHeight, RED);
    }

    sliderY += spacingY;

    // Регулятор частоти оновлення інтерфейсу (мс)
    // int Cam5 =
    Gui_CamSwitch_Channel(5, Terminus12x6_font, TerminusBold18x10_font,
                          sliderX + 65, sliderY,
                          "Refresh\nRate (ms)",
                          NULL/* TextFormat("%d", (int)oscData->refresh_rate_ms)*/,
                          knob_radius,
                          &oscData->refresh_rate_ms, 5.0f, 55.0f, true, WHITE);
    // if(Cam5) oscData->refresh_rate_ms = roundf(oscData->refresh_rate_ms / 0.50f) * 0.50f; // крок 5.0

    // **Пояснення:**
    // - Мета отримати інкремент слайдеру 5
    // - Регулятор повертає значення з плаваючою точкою.
    // - Ми додаємо 2.5 для коректного округлення при діленні на 5.
    // - Приводимо до int для цілочисельного ділення і множимо назад на 5 — отримуємо крок 5.
    // - Таким чином користувач бачить і змінює значення слайдера з кроком 5.
    // oscData->refresh_rate_ms = ((int)(oscData->refresh_rate_ms + 2.5f) / 5) * 5;

    static float old_refresh_rate_ms;
    char command[24] = "Rate:";  // достатній розмір буфера

    if (old_refresh_rate_ms != oscData->refresh_rate_ms)
    {
        old_refresh_rate_ms = oscData->refresh_rate_ms;
        send_command(oscData, command, sizeof(command), (int)oscData->refresh_rate_ms);
    }

    // Регулятор горизонтального зміщення тригера
    int Cam6 = Gui_Knob_Channel(6, Terminus12x6_font, TerminusBold18x10_font,
                                sliderX + 234, sliderY,
                                "Trigger\noffset X",
                                NULL/*TextFormat("%d", (int)oscData->trigger_offset_x)*/,
                                knob_radius,
                                &oscData->trigger_offset_x, 0, 500, true, WHITE);
    if(Cam6) oscData->trigger_offset_x = roundf(oscData->trigger_offset_x / 5.0f) * 5.0f; // крок 5.0
    // if(Cam6) oscData->trigger_offset_x = ((int)(oscData->trigger_offset_x + 2.5f) / 5) * 5;

    sliderY += spacingY / 2 + 5;

    Color colorTriggerMode; // Встановлюємо колір якщо тригер активований
    if (Ch->trigger_active)
        colorTriggerMode = activeColor;
    else
        colorTriggerMode = LIGHTGRAY;

    Rectangle trgBounds = { sliderX-5, sliderY-2, 310, 34 };
    DrawRectangleRec(trgBounds, ChangeSaturation(colorTriggerMode, 0.25f));
    DrawRectangleRec(trgBounds, Fade(GRAY, 0.5f));

    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                 &Ch->trigger_active,
                 TerminusBold24x12_font, "Activate\ntrigger", NULL, activeColor);

    // радіокнопоки тригера
    Rectangle radioBounds = { sliderX + 200, sliderY, 3 * 30 + 2 * 5, 30 }; // 3 кнопки 30px + 2 проміжки по 5px

    bool showButtons = 1; // Slider mode 0, Spinner mode 1.
    // Рівень гістерезису
    float Min = -1.0f, Max = 1.0f;
    Gui_SliderSpinner(3, sliderX + 115, sliderY+radioBounds.height/2, 120, 24, NULL, NULL,
                      &Ch->trigger_hysteresis_px, &Min, &Max,
                      0.1f, GUI_SPINNER_FLOAT, GUI_SPINNER_HORIZONTAL,
                      colorTriggerMode, Terminus12x6_font, spacing, showButtons);

    int newSelection = Gui_RadioButtons_Row(radioBounds, TerminusBold24x12_font, radiobuttonItems, 3,
                                            Ch->trigger_edge,
                                            colorTriggerMode, 30, 5);

    if (newSelection != Ch->trigger_edge) {
        Ch->trigger_edge = newSelection;

        // Відправка команди на пристрій
        char cmd[32] = "TriggerEdge:";
        send_command(oscData, cmd, sizeof(cmd), newSelection);
    }

    sliderY += spacingY /2 - 25;

    // Перемикач тестового сигналу
    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                 &oscData->test_signal,
                 TerminusBold24x12_font ,"Тестовий\nсигнал", NULL, DARKGRAY);

    static bool test_signal;
    char command2[24] = "Test signal:";  // достатній розмір буфера

    if (test_signal != oscData->test_signal)
    {
        test_signal = oscData->test_signal;
        send_command(oscData, command2, sizeof(command2), (int)oscData->test_signal);
    }

    // sliderY += spacingY / 2;

    // Перемикач перемотки сигналу
    Gui_CheckBox((Rectangle){sliderX + 40, sliderY, 30, 30},
                 &oscData->movement_signal,
                 TerminusBold24x12_font ,"Прокручування\nсигналу\nMovement", NULL, DARKGRAY);

    // sliderY += spacingY / 2;

    // Перемикач реверсу напрямку малювання сигналу
    Gui_CheckBox((Rectangle){sliderX+80, sliderY, 30, 30},
                 &oscData->reverse_signal,
                 TerminusBold24x12_font ,"Реверс\nсигналу", NULL, DARKGRAY);

    Gui_CheckBox((Rectangle){sliderX+120, sliderY, 30, 30},
                 &oscData->dynamic_buffer_mode,
                 TerminusBold24x12_font ,"Динамічний\nбуфер\npoints_to_display",
                 /*TextFormat(": %d",oscData->points_to_display)*/NULL, DARKGRAY);

    int intMin = 100, intMax = 10000;
    Gui_SliderSpinner(0, sliderX+240, sliderY+15, 125, 25, NULL, NULL,
                      &oscData->points_to_display, &intMin, &intMax,
                      100, GUI_SPINNER_INT, GUI_SPINNER_HORIZONTAL,
                      BLUE, TerminusBold18x10_font, spacing, showButtons);

    static bool last_dynamic_mode = false;
    if (oscData->dynamic_buffer_mode != last_dynamic_mode) {
        oscData->points_to_display = oscData->dynamic_buffer_mode ? oscData->points_to_display : 10000;
        setup_channel_buffers(oscData);
        last_dynamic_mode = oscData->dynamic_buffer_mode;
    }

    static int last_buffer_size;
    if (oscData->dynamic_buffer_mode && oscData->points_to_display != last_buffer_size) {
        setup_channel_buffers(oscData);
        last_buffer_size = oscData->points_to_display;
    }

    Min = 0.0f; Max = 550.0f;
    Gui_SliderSpinner(1, 325, WORKSPACE_HEIGHT+35, 550, 12, NULL, NULL,
                      &oscData->trigger_offset_x, &Min, &Max,
                      5.0f, GUI_SPINNER_FLOAT, GUI_SPINNER_HORIZONTAL,
                      LIGHTGRAY, Terminus12x6_font, spacing, !showButtons);

    Min = 250.0f, Max = -250.f;
    Gui_SliderSpinner(2, 25, 300, 10, 500, NULL, NULL,
                      &Ch->offset_y, &Min, &Max,
                      5.0f, GUI_SPINNER_FLOAT, GUI_SPINNER_VERTICAL,
                      activeColor, Terminus12x6_font, spacing, !showButtons);

    int sliderWidth = 10;
    int sliderHeight = 500;
    Rectangle sliderBounds = { sliderX - 35, 50, sliderWidth, sliderHeight };
    RegisterSlider(0, sliderBounds, &oscData->channels[0].offset_y, 250.0f, -250.0f, true, YELLOW, NULL, NULL);
    RegisterSlider(1, sliderBounds, &oscData->channels[1].offset_y, 250.0f, -250.0f, true, GREEN, NULL, NULL);
    RegisterSlider(2, sliderBounds, &oscData->channels[2].offset_y, 250.0f, -250.0f, true, RED, NULL, NULL);
    RegisterSlider(3, sliderBounds, &oscData->channels[3].offset_y, 250.0f, -250.0f, true, SKYBLUE, NULL, NULL);

    // Централізована функція, яка обробляє взаємодію і малює всі слайдери
    UpdateSlidersAndDraw(TerminusBold18x10_font, 2);
    // Синхронізуємо активний регулятор зі слайдером (встановлює крок слайдера для активного каналу)
    Ch->offset_y = roundf(Ch->offset_y / 5.0f) * 5.0f; // кратність 5.0f

    // Оновлюємо та реєструємо стан слайдерів
    Rectangle Bounds = { sliderX - 50, 50, 6, sliderHeight };
    // RegisterCircleKnobSlider(0, Bounds, &Ch->scale_y, 0.2f, 2.20f, true, WHITE, NULL, NULL);
    RegisterCircleKnobSlider(0, Bounds, &oscData->channels[0].scale_y, 0.2f, 2.20f, true, YELLOW, NULL, NULL);
    RegisterCircleKnobSlider(1, Bounds, &oscData->channels[1].scale_y, 0.2f, 2.20f, true, GREEN, NULL, NULL);
    RegisterCircleKnobSlider(2, Bounds, &oscData->channels[2].scale_y, 0.2f, 2.20f, true, RED, NULL, NULL);
    RegisterCircleKnobSlider(3, Bounds, &oscData->channels[3].scale_y, 0.2f, 2.20f, true, SKYBLUE, NULL, NULL);

    // Централізована функція, яка обробляє взаємодію і малює всі слайдери
    UpdateCircleKnobSlidersAndDraw(TerminusBold18x10_font, 2);
    // Синхронізуємо активний регулятор зі слайдером (встановлює крок слайдера для активного каналу)
    Ch->scale_y = roundf(Ch->scale_y / 0.02f) * 0.02f; // кратність 0.02f
}

void send_command(OscData *data, char* command, size_t buffer_size, int number)
{
    //snprintf(command, command_size, "Rate: %d", number);
    size_t len = strlen(command);

    // Перед викликом snprintf добре перевірити,
    // що buffer_size > len щоб уникнути переповнення:
    if (buffer_size > len) {
        snprintf(command + len, buffer_size - len, "%d", number);
    } else {
        // Обробка помилки: недостатньо місця в буфері
        fprintf(stderr, "Buffer overflow prevented in send_command\n");
        return;
    }

    if (buffer_size > len) {
        command[len] = '\0';
    }

    /*
    // Якщо потрібно, щоб між текстом і числом був пробіл,
    // двокрапка або інший символ, додай його перед snprintf:
    if (buffer_size > len) {
        command[len] = ':';  // або ':', або інший символ
        command[len + 1] = ' ';  // або ':', або інший символ
        command[len + 2] = '\0';
        len += 2;
    }
    */

    snprintf(command + len, buffer_size - len, "%d\n", number);
    // printf("command: %s\n", command);
    // printf("number: %d\n", number);


    len = strlen(command);
    write_usb_device(data, command, len);
}

void write_usb_device(OscData *data, unsigned char* str, size_t len)
{
    // RS232_cputs(data->comport_number, str); // не працює ???
    // size_t len = strlen(str);
    RS232_SendBuf(data->comport_number, str, len);
    // printf("%s\n", str);

    // Код перевірки парсингу
    int new_rate;
    if (strncmp(str, "Rate:", 5) == 0)
    {
        new_rate = atoi(str + 5);
        // set_adc_sampling_rate(new_rate);
    }

    /*
    if (strncmp(str, "Rate:", 5) == 0)
    {
        new_rate = atoi(str + 6);
        // set_adc_sampling_rate(new_rate);
        // printf("new_rate %d \n", new_rate);
    }*/

    /*
    int parse;
    if (strncmp(str, "Test signal:", 12) == 0)
    {
        parse = atoi(str + 12);
        printf("parse %d\n", parse);
    }
    */

    printf("%s", str);
    // printf("new_rate: %d\n\n", new_rate);
}

