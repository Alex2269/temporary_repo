// file find_usb_device.h

#include "main.h"
#include "rs232.h"
#include "find_usb_device.h"
#include <stdint.h>

void find_usb_device(OscData *oscData)
{
    // Автоматичне відкриття COM-порту
    int comport_to_find = 0;
    char mode[] = {'8','N','1',0};
    bool port_found = false;

    while (comport_to_find < 38) {
        if (RS232_OpenComport(comport_to_find, 115200, mode, 0) == 0) {
            oscData->comport_number = comport_to_find;
            printf("Автоматично відкрито COM порт: %d\n", oscData->comport_number);
            port_found = true;
            if (comport_to_find == 24) {
                strcpy(oscData->com_port_name_input, "/dev/ttyACM0");
            } else {
                sprintf(oscData->com_port_name_input, "Port %d", comport_to_find);
            }
            break;
        }
        comport_to_find++;
        if (comport_to_find == 37) {
            sleep(1);
        }
    }

    if (!port_found) {
        printf("Не вдалося автоматично відкрити жоден COM порт.\n");
        strcpy(oscData->com_port_name_input, "/dev/ttyACM0");
    }
}

