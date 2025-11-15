
#ifndef __SystemClock_Config_H
#define __SystemClock_Config_H

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

void SystemInit(void);
void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* __SystemClock_Config_H */
