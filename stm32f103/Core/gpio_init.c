
#include "gpio_init.h"

// // Перелічення режимів роботи пінів
// #define GPIO_MODE_INPUT          0x00U // Вхід (режим за замовчуванням)
// #define GPIO_MODE_OUTPUT_PP      0x01U // Логічний вихід (push-pull)
// #define GPIO_MODE_AF_PP          0x02U // Альтернативний функціональний вихід (push-pull)
// #define GPIO_MODE_ANALOG         0x03U // Аналоговий вхід

// Ініціалізація піну GPIO з параметром mode
uint32_t gpio_init(GPIO_TypeDef *GPIOx, uint16_t pin, uint32_t mode)
{
    // Перевіряємо, чи коректно передано пін (0-15)
    if (pin > 15) {
        return 1; // Помилка, пін знаходиться поза допустимими межами
    }

    if(GPIOx == GPIOA) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // Увімкнути тактування GPIOA
    if(GPIOx == GPIOB) RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // Увімкнути тактування GPIOB
    if(GPIOx == GPIOC) RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // Увімкнути тактування GPIOC
    if(GPIOx == GPIOD) RCC->APB2ENR |= RCC_APB2ENR_IOPDEN; // Увімкнути тактування GPIOD

    // Визначаємо регістр для конфігурації пінів (CRL для пінів 0-7, CRH для пінів 8-15)
    volatile uint32_t *config_register = (pin < 8) ? &GPIOx->CRL : &GPIOx->CRH;
    uint32_t shift = (pin % 8) * 4; // Визначаємо позицію піну в регістрі (4 біти на пін)

    // Маска для очищення поточної конфігурації
    uint32_t mask = 0xF << shift;
    
    // Очищаємо поточну конфігурацію піну
    *config_register &= ~mask;

    // Конфігуруємо пін в залежності від бажаного режиму
    switch (mode) {
        case GPIO_MODE_INPUT: // Режим входу
            *config_register |= (0x0 << shift); // Режим для вхідного піну
            break;

        case GPIO_MODE_OUTPUT_PP: // Логічний вихід (push-pull)
            *config_register |= (0x1 << shift); // Режим для виходу (push-pull)
            GPIOx->ODR &= ~(1 << pin); // Встановлюємо низький рівень за замовчуванням
            break;

        case GPIO_MODE_AF_PP: // Альтернативний функціональний вихід (push-pull)
            *config_register |= (0x2 << shift); // Режим для альтернативного виходу
            break;

        case GPIO_MODE_ANALOG: // Аналоговий вхід
            *config_register |= (0x3 << shift); // Режим для аналогового входу
            break;

        default:
            return 2; // Помилка, невідомий режим
    }

    // У випадку виходу або альтернативного функціонального режиму налаштовуємо швидкість
    if (mode == GPIO_MODE_OUTPUT_PP || mode == GPIO_MODE_AF_PP) {
        // Встановлюємо максимальну швидкість (50 MHz) для вихідних пінів
        uint32_t speed = 0x3 << (shift / 4); // Встановлюємо максимальну швидкість (50 MHz)
        *config_register |= speed;
    }

    return 0; // Успішна ініціалізація
}

// Функція для запису в пін
uint32_t gpio_write_pin(GPIO_TypeDef *GPIOx, uint16_t PINx, uint8_t state)
{
    // Перевіряємо стан (0 або 1), якщо state == 1 - встановлюємо пін в високий рівень, якщо state == 0 - в низький
    if(state == 1)
    {
        // Встановлюємо пін в високий рівень (логічна 1)
        GPIOx->BSRR = (1 << PINx);  // Встановлюємо пін на високий рівень, зсуваючи одиницю на номер піна
    }
    else
    {
        // Встановлюємо пін в низький рівень (логічна 0)
        GPIOx->BRR = (1 << PINx);   // Встановлюємо пін на низький рівень, зсуваючи одиницю на номер піна
    }
    return 0;  // Повертаємо 0, що означає успішне виконання
}

// Функція для читання стану піну
uint8_t gpio_read_pin(GPIO_TypeDef *GPIOx, uint16_t PINx)
{
    // Перевіряємо значення на піні:
    // Якщо значення на піні встановлене на 1, повертаємо 1, інакше 0.
    if(GPIOx->IDR & (1 << PINx))  // IDR - Input Data Register
    {
        return 1;  // Пін високий (логічна 1)
    }
    else
    {
        return 0;  // Пін низький (логічна 0)
    }
}

// Функція для переключення стану піну
uint32_t gpio_toggle_pin(GPIO_TypeDef *GPIOx, uint16_t PINx)
{
    // Перевіряємо стан піну, щоб переключити його на протилежний:
    if(GPIOx->IDR & (1 << PINx))  // Якщо пін у високому стані (логічна 1)
    {
        // Якщо пін високий, встановлюємо його в низький рівень
        GPIOx->BRR = (1 << PINx);   // Скидаємо пін на 0
    }
    else
    {
        // Якщо пін низький, встановлюємо його в високий рівень
        GPIOx->BSRR = (1 << PINx);  // Встановлюємо пін на 1
    }
    return 0;  // Повертаємо 0, що означає успішне виконання
}

