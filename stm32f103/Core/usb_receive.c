// file usb_receive.c
#include <string.h>
#include "gpio_init.h"
#include "usb_receive.h"

uint16_t new_rate;
uint16_t test_signal;

void trim_string(char* str)
{
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n' || str[len-1] == ' '))
    {
        str[len-1] = '\0';
        len--;
    }
}

// Функція обробки прийнятих даних
void USB_CDC_RxHandler(uint8_t* data, uint32_t length)
{
    char buffer[128];
    if (length >= sizeof(buffer))
        length = sizeof(buffer) - 1;

    memcpy(buffer, data, length);
    buffer[length] = '\0';

    trim_string(buffer);

    // printf("%s\n", buffer);

    if (strcmp(buffer, "LED ON") == 0)
    {
        gpio_write_pin(GPIOC, 13, 0); // Ввімкнути світлодіод
        // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        printf("LED turned ON\n");
    }

    if (strcmp(buffer, "LED OFF") == 0)
    {
        gpio_write_pin(GPIOC, 13, 1); // Вимкнути світлодіод
        // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        printf("LED turned OFF\n");
    }

    if (strncmp(buffer, "Rate:", 5) == 0)
    {
        new_rate = atoi(buffer + 5);
        // set_adc_sampling_rate(new_rate);
        // printf("new_rate %d \n", new_rate);
    }

    if (strncmp(buffer, "Test signal:", 12) == 0)
    {
        test_signal = atoi(buffer + 12);
    }

    else
    {
       ; // printf("Unknown command\n");
    }

    for(uint16_t i=0; i < length; i++) { buffer[i] = '\0'; }
}
