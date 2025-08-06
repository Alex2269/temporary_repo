
#ifndef __STM32F1xx_LL_UTILS_H
#define __STM32F1xx_LL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx.h"


#define LL_MAX_DELAY                  0xFFFFFFFFU

void        LL_Init1msTick(uint32_t HCLKFrequency);
void        LL_mDelay(uint32_t Delay);
void        LL_SetSystemCoreClock(uint32_t HCLKFrequency);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_LL_UTILS_H */
