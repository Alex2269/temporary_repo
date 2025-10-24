// gui_control_panel.c

#include "raylib.h"
#include "main.h" // для OscData, MAX_CHANNELS
#include "gui_control_panel.h"
#include "button.h"
#include "guicheckbox.h"
// #include "sliders.h"
// #include "slider_widget_ellipse.h"
#include "slider_widget_circle.h"
#include "slider_widget.h"
#include "cam_switch.h"
#include "knob_gui.h"
#include "gui_spinner.h"
#include "setup_channel_buffers.h"
#include "rs232.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

extern int LineSpacing;    // Відступ між рядками тексту
extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

// початкове значення (тимчасова змінна для спінера, бо спінер приймає адрес змінної float)
float cuont_points=500.0f;

// #include "gui_radiobutton.h"
#include "gui_radiobutton_row.h"

// Статус перетягування
static bool dragging_trigger_line = false;
void GuiControlPanelRender(OscData *data);
float trigger_y_position = 0.0f;  // Початкове логічне значення

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
                                &Ch->scale_y, 0.1f, 1.10f, true, activeColor);

    if(Cam0) Ch->scale_y = roundf(Ch->scale_y / 0.1) * 0.1;
    if(Cam0) printf("scale: %0.2f\n", Ch->scale_y);
    // Ch->scale_y = ((int)(Ch->scale_y + 2.5f) / 5) * 5;
    static float old_scale_y;
    if (old_scale_y != Ch->scale_y) old_scale_y = Ch->scale_y;

    // Зміщення по вертикалі
    int Cam1 = Gui_Knob_Channel(1, Terminus12x6_font, TerminusBold18x10_font,
                                sliderX + 234, sliderY,
                                "Vertical\noffset",
                                NULL/*TextFormat("%d", (int)Ch->offset_y)*/,
                                knob_radius,
                                &Ch->offset_y, -625.0f, 625.0f, true, activeColor);

    if(Cam1) Ch->offset_y = roundf(Ch->offset_y / 25) * 25; // кратність 25 px
    static float old_offset_y;
    if (old_offset_y != Ch->offset_y) old_offset_y = Ch->offset_y;

    sliderY += spacingY;

    // Рівень тригера
    Gui_Knob_Channel(2, Terminus12x6_font, TerminusBold18x10_font,
                     sliderX + 65, sliderY,
                     "Trigger\nlevel",
                     NULL/*TextFormat("%0.1f", Ch->trigger_level)*/,
                     knob_radius,
                     &Ch->trigger_level, 0.0f, 1.0f, true, activeColor);

    // зберігаємо позицію тригера
    trigger_y_position = Ch->trigger_level;

    GuiControlPanelRender(oscData);

    // Рівень гістерезису
    Gui_Knob_Channel(3, Terminus12x6_font, TerminusBold18x10_font,
                     sliderX + 234, sliderY,
                     "Trigger\nhysteresis",
                     NULL/*TextFormat("%0.1f", Ch->trigger_hysteresis_px)*/,
                     knob_radius,
                     &Ch->trigger_hysteresis_px, -1.0f, 1.0f, true, activeColor);

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
    Gui_Knob_Channel(4, Terminus12x6_font, TerminusBold18x10_font,
                     sliderX + 65, sliderY,
                     "Refresh\nRate (ms)",
                     NULL/* TextFormat("%d", (int)oscData->refresh_rate_ms)*/,
                     knob_radius,
                     &oscData->refresh_rate_ms, 5.0f, 55.0f, true, WHITE);

    // float fMin = 5.0f, fMax = 55.0f;
    // Gui_Spinner(4, sliderX + 65, 75, 60, 12, NULL, NULL,
    //             &oscData->refresh_rate_ms, &fMin, &fMax,
    //             5.0f, GUI_SPINNER_FLOAT,
    //             DARKGRAY, font12, spacing);

    // **Пояснення:**
    // - Мета отримати інкремент слайдеру 5
    // - `GuiSlider` повертає значення з плаваючою точкою.
    // - Ми додаємо 2.5 для коректного округлення при діленні на 5.
    // - Приводимо до int для цілочисельного ділення і множимо назад на 5 — отримуємо крок 5.
    // - Таким чином користувач бачить і змінює значення слайдера з кроком 5.
    oscData->refresh_rate_ms = ((int)(oscData->refresh_rate_ms + 2.5f) / 5) * 5;

    static float old_refresh_rate_ms;
    char command[24] = "Rate:";  // достатній розмір буфера

    if (old_refresh_rate_ms != oscData->refresh_rate_ms)
    {
        old_refresh_rate_ms = oscData->refresh_rate_ms;
        send_command(oscData, command, sizeof(command), (int)oscData->refresh_rate_ms);
    }

    // Регулятор горизонтального зміщення тригера
    Gui_Knob_Channel(5, Terminus12x6_font, TerminusBold18x10_font,
                     sliderX + 234, sliderY,
                     "Trigger\noffset X",
                     NULL/*TextFormat("%d", (int)oscData->trigger_offset_x)*/,
                     knob_radius,
                     &oscData->trigger_offset_x, 0, 1000, true, WHITE);

    sliderY += spacingY / 2 + 15;

    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                &Ch->trigger_active,
                TerminusBold24x12_font, "Activate\ntrigger", "Trigger", activeColor);

    // радіокнопоки тригера
    Rectangle radioBounds = { sliderX + 200, sliderY, 3 * 30 + 2 * 5, 30 }; // 3 кнопки 30px + 2 проміжки по 5px

    Color colorTriggerMode; // Встановлюємо колір якщо тригер активований
    if (Ch->trigger_active)
        colorTriggerMode = activeColor;
    else
        colorTriggerMode = LIGHTGRAY;

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
    bool changedInt = Gui_Spinner(0, sliderX+240, sliderY+15, 125, 25, NULL, NULL,
                                     &oscData->points_to_display, &intMin, &intMax,
                                     100, GUI_SPINNER_INT, BLUE, TerminusBold18x10_font, spacing);

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

    // Автоматичне підключення (тимчасово вимкнено)
    /* Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30}, TerminusBold24x12_font , "Auto Connect", &oscData->auto_connect);
    sliderY += spacingY; */

    // Gui_Slider((Rectangle){25, screenHeight-15, 600, 10},
    //            TerminusBold24x12_font, NULL/*"Sl"*/, NULL/*"sl"*/,
    //            &oscData->trigger_offset_x,
    //            0.0f, 1000.0f, 0, WHITE);

    float Min = 0.0f, Max = 1000.0f;
    Gui_Spinner(1, 325, WORKSPACE_HEIGHT+35, 600, 12, NULL, NULL,
                &oscData->trigger_offset_x, &Min, &Max,
                5.0f, GUI_SPINNER_FLOAT,
                DARKGRAY, Terminus12x6_font, spacing);

    // int sliderWidth = 10;
    // int sliderHeight = 550;
    // Rectangle sliderBounds = { sliderX - 25, 20, sliderWidth, sliderHeight };
    // Gui_Slider(sliderBounds, TerminusBold18x10_font,
    //         NULL/*TextFormat("Масштабування CH%d\nпо вертикалі", ch + 1)*/,
    //         NULL,
    //         &Ch->offset_y, 550.0f, 0.0f, true, activeColor);

    int sliderWidth = 10;
    int sliderHeight = 560;
    Rectangle sliderBounds = { sliderX - 35, 20, sliderWidth, sliderHeight };
    // Gui_SliderEx(0, sliderBounds, font18, NULL, NULL, &oscData->channels[0].offset_y, 700.0f, 0.0f, true, YELLOW);
    // Gui_SliderEx(1, sliderBounds, font18, NULL, NULL, &oscData->channels[1].offset_y, 700.0f, 0.0f, true, GREEN);
    // Gui_SliderEx(2, sliderBounds, font18, NULL, NULL, &oscData->channels[2].offset_y, 700.0f, 0.0f, true, RED);
    // Gui_SliderEx(3, sliderBounds, font18, NULL, NULL, &oscData->channels[3].offset_y, 700.0f, 0.0f, true, SKYBLUE);

    // RegisterSlider(0, sliderBounds, &Ch->offset_y, 700.0f, 0.0f, true, WHITE, NULL, NULL);
    RegisterSlider(0, sliderBounds, &oscData->channels[0].offset_y, 625.0f, -625.0f, true, YELLOW, NULL, NULL);
    RegisterSlider(1, sliderBounds, &oscData->channels[1].offset_y, 625.0f, -625.0f, true, GREEN, NULL, NULL);
    RegisterSlider(2, sliderBounds, &oscData->channels[2].offset_y, 625.0f, -625.0f, true, RED, NULL, NULL);
    RegisterSlider(3, sliderBounds, &oscData->channels[3].offset_y, 625.0f, -625.0f, true, SKYBLUE, NULL, NULL);

    // Централізована функція, яка обробляє взаємодію і малює всі слайдери
    UpdateSlidersAndDraw(TerminusBold18x10_font, 2);

    // Оновлюємо та реєструємо стан слайдерів
    Rectangle Bounds = { sliderX - 50, 20, 6, sliderHeight };
    // RegisterCircleKnobSlider(0, Bounds, &Ch->scale_y, 0.10f, 2.0f, true, WHITE, NULL, NULL);
    RegisterCircleKnobSlider(0, Bounds, &oscData->channels[0].scale_y, 0.10f, 2.0f, true, YELLOW, NULL, NULL);
    RegisterCircleKnobSlider(1, Bounds, &oscData->channels[1].scale_y, 0.10f, 2.0f, true, GREEN, NULL, NULL);
    RegisterCircleKnobSlider(2, Bounds, &oscData->channels[2].scale_y, 0.10f, 2.0f, true, RED, NULL, NULL);
    RegisterCircleKnobSlider(3, Bounds, &oscData->channels[3].scale_y, 0.10f, 2.0f, true, SKYBLUE, NULL, NULL);

    // Rectangle Bound = { 25, WORKSPACE_HEIGHT + 35, 600, 6 };
    // RegisterCircleKnobSlider(4, Bound, &oscData->trigger_offset_x,
    //                          0.0f, 1000.0f, false, WHITE, "Trigger offset X", NULL);

    // Централізована функція, яка обробляє взаємодію і малює всі слайдери
    UpdateCircleKnobSlidersAndDraw(TerminusBold18x10_font, 2);

    // Аналогічно інші слайдери для offset_y, trigger_level і т.д.
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

void GuiControlPanelRender(OscData *oscData)
{
    // Отримати активний канал та його налаштування
    int ch = oscData->active_channel;
    Color activeColor = channel_colors[ch];
    ChannelSettings *Ch = &oscData->channels[ch];

    // Задаємо фіксовані координати лінії тригера по горизонталі (X)
    int x_start = 25;
    int x_end = 75;

    // Колір для лінії та ручки
    Color trigger_color = activeColor;

    // Обчислення поточної вертикальної позиції тригера в пікселях для малювання
    int pixel_y = (int)(Ch->offset_y - (Ch->trigger_level * WORKSPACE_HEIGHT) * Ch->scale_y);

    // Задаємо радіус для логіки захоплення (більший - зона кліку ширша)
    int handle_capture_radius = 12;

    // І радіус для малювання ручки (менший - щоб візуально ручка залишалась компактною)
    int handle_draw_radius = 5;

    // ОБРОБКА ПОЧАТКУ ПЕРЕТЯГУВАННЯ
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        int mouseX = GetMouseX();
        int mouseY = GetMouseY();

        // Перевірка, чи курсор у межах збільшеної зони захоплення навколо ручки (коло з радіусом handle_capture_radius)
        int dx = mouseX - x_start;
        int dy = mouseY - pixel_y;

        if (dx*dx + dy*dy <= handle_capture_radius * handle_capture_radius)
        {
            dragging_trigger_line = true;
        }
    }

    // ОБРОБКА ПЕРЕТЯГУВАННЯ
    if (dragging_trigger_line)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            int mouseY = GetMouseY();

            // Обмеження вертикального руху в межах робочої області
            int top_limit = 5;
            int bottom_limit = 595;

            if (mouseY < top_limit) mouseY = top_limit;
            if (mouseY > bottom_limit) mouseY = bottom_limit;

            // Конвертація піксельної координати у логічну позицію тригера (0..1)
            trigger_y_position = (Ch->offset_y - (float)mouseY) / Ch->scale_y / WORKSPACE_HEIGHT;

            // Оновлення рівня тригера в каналі
            Ch->trigger_level = trigger_y_position;
        }
        else
        {
            // Кінець перетягування при відпусканні кнопки миші
            dragging_trigger_line = false;
        }
    }
    else
    {
        // Синхронізація локальної позиції з поточним значенням рівня тригера
        trigger_y_position = Ch->trigger_level;
    }

    // Оновлення піксельної позиції для малювання після впливу перетягування
    pixel_y = (int)(Ch->offset_y - (trigger_y_position * WORKSPACE_HEIGHT) * Ch->scale_y);

    // МАЛЮВАННЯ ЛІНІЇ ТРИГЕРА (горизонтальна, зафіксована по X)
    DrawLine(x_start, pixel_y, x_end, pixel_y, trigger_color);

    // МАЛЮВАННЯ РУЧКИ (маркер) із меншим радіусом для збереження звичного вигляду
    DrawCircle(x_start, pixel_y, handle_draw_radius, trigger_color);
}

