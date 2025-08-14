#ifndef USB_RECEIVE_H
#define USB_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_cdc_if.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"

void USB_CDC_RxHandler(uint8_t* data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* USB_RECEIVE_H */
