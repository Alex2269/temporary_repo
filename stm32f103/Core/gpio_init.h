
#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx.h" // підключаємо CMSIS бібліотеку для STM32F103

// Перелічення режимів роботи пінів
#define GPIO_MODE_INPUT          0x00U // Вхід (режим за замовчуванням)
#define GPIO_MODE_OUTPUT_PP      0x01U // Логічний вихід (push-pull)
#define GPIO_MODE_AF_PP          0x02U // Альтернативний функціональний вихід (push-pull)
#define GPIO_MODE_ANALOG         0x03U // Аналоговий вхід

// Ініціалізація піну GPIO з параметром mode
uint32_t gpio_init(GPIO_TypeDef *GPIOx, uint16_t pin, uint32_t mode);
// Функція для запису в пін
uint32_t gpio_write_pin(GPIO_TypeDef *GPIOx, uint16_t PINx, uint8_t state);
// Функція для читання стану піну
uint8_t gpio_read_pin(GPIO_TypeDef *GPIOx, uint16_t PINx);
// Функція для переключення стану піну
uint32_t gpio_toggle_pin(GPIO_TypeDef *GPIOx, uint16_t PINx);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_INIT_H */
