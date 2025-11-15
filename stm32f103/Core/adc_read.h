#ifndef adc_read_H_
#define adc_read_H_

#include "stm32f103xb.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f103xb.h"

// === Функція для ініціалізації пінів для аналогових входів ===
void Init_ADC_Pin(GPIO_TypeDef *GPIOx, uint32_t pin);
// === Функція для запуску калібрування ADC ===
void ADC_StartCalibration(ADC_TypeDef *ADCx);
// === Функція ініціалізації ADC ===
void Init_ADC(ADC_TypeDef *ADCx);
// === Функція для отримання значення з ADC ===
uint16_t Read_ADC(ADC_TypeDef *ADCx, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* adc_read_H_ */
