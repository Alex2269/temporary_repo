#include "adc_read.h"
#include "stm32f103xb.h"

// === Функція для ініціалізації пінів для аналогових входів ===
void Init_ADC_Pin(GPIO_TypeDef *GPIOx, uint32_t pin) {
    // Увімкнути тактування GPIO
    if (GPIOx == GPIOA) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    if (GPIOx == GPIOB) RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // Налаштувати пін як аналоговий вхід
    if (pin < 8) {  // Конфігурація для пінів 0-7 (LOW Register)
        GPIOx->CRL &= ~(0xF << (pin * 4));  // 0b0000 (Аналоговий режим)
    } else {  // Конфігурація для пінів 8-15 (HIGH Register)
        GPIOx->CRH &= ~(0xF << ((pin - 8) * 4));
    }
}

// === Функція для запуску калібрування ADC ===
void ADC_StartCalibration(ADC_TypeDef *ADCx) {
    ADCx->CR2 |= ADC_CR2_ADON; // Увімкнути АЦП
    for (volatile uint32_t i = 0; i < 10000; i++); // Затримка для стабілізації

    ADCx->CR2 |= ADC_CR2_CAL; // Запустити калібрування
    while (ADCx->CR2 & ADC_CR2_CAL); // Чекати завершення калібрування
}

// === Функція ініціалізації ADC ===
void Init_ADC(ADC_TypeDef *ADCx) {
    // Увімкнути тактування ADC
    if (ADCx == ADC1) RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    if (ADCx == ADC2) RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;

    // Увімкнути тактування альтернативних функцій (необхідно для ADC)
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    // Налаштування ADC
    ADCx->CR1 &= ~ADC_CR1_SCAN; // Вимкнути режим сканування
    ADCx->CR2 &= ~ADC_CR2_CONT; // Вимкнути безперервний режим (одиночне вимірювання)

    // Встановити вибраний канал для конверсії
    ADCx->SQR3 = 0; // Перший канал у черзі
    ADCx->SQR1 &= ~ADC_SQR1_L; // Один канал у послідовності

    // Увімкнути ADC та виконати калібрування
    ADC_StartCalibration(ADCx);
}

// === Функція для отримання значення з ADC ===
uint16_t Read_ADC(ADC_TypeDef *ADCx, uint8_t channel) {
    // Встановити вибраний канал для конверсії
    ADCx->SQR3 = channel;  // Вибір каналу в першій позиції черги
    ADCx->SQR1 &= ~ADC_SQR1_L;  // Вимірюємо лише один канал

    // Встановити час вибірки для вибраного каналу (28.5 циклів, можна змінити)
    if (channel < 10) {
        ADCx->SMPR2 |= (0b101 << (channel * 3));  // SMPR2 для каналів 0-9
    } else {
        ADCx->SMPR1 |= (0b101 << ((channel - 10) * 3));  // SMPR1 для каналів 10-17
    }

    // Запуск ADC (двічі для початку конверсії)
    ADCx->CR2 |= ADC_CR2_ADON;
    ADCx->CR2 |= ADC_CR2_ADON;

    // Чекати завершення конверсії
    while (!(ADCx->SR & ADC_SR_EOC));

    // Зчитати значення (12 біт)
    uint16_t result = ADCx->DR;

    return result;
}

