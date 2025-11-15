#include "SystemClock_Config.h"

// Функція ініціалізації системи (запускається при старті програми)
void SystemInit(void)
{
  // Якщо вказано адресу таблиці векторів (для користувача), то вона буде ініціалізована
  #if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = 0x08000000UL;  // Адреса таблиці векторів (може бути змінена користувачем)
  #endif
}

// Функція налаштування системного тактового генератора
void SystemClock_Config(void)
{
    /* Увімкнути буфер передзапису */
    FLASH->ACR |= FLASH_ACR_PRFTBE;
    // Встановлення латентності флеш-пам'яті на 2 такти для роботи з тактовими частотами вище 24 МГц
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    // Увімкнути зовнішній осцилятор HSE (High-Speed External)
    RCC->CR |= RCC_CR_HSEON;
    // Чекати, поки HSE стабілізується
    while (!(RCC->CR & RCC_CR_HSERDY));

    // Налаштування AHB (72 МГц), APB1 (36 МГц), APB2 (72 МГц)
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = SYSCLK / 1 = 72 МГц
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = SYSCLK / 2 = 36 МГц
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 = SYSCLK / 1 = 72 МГц

    // Очистити налаштування PLL перед новою конфігурацією
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL); // Скидаємо налаштування PLL

    // Встановлюємо HSE як джерело для PLL (HSE / 1 × 9 = 72 МГц)
    RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE; // PLLXTPRE: дільник HSE для PLL, 0: HSE не ділиться
    RCC->CFGR |= RCC_CFGR_PLLSRC; // PLLSRC: джерело годинника для PLL, 1: вибрано HSE як вхідний годинник PLL
    RCC->CFGR |= RCC_CFGR_PLLMULL9; // PLLMUL: множник для PLL, 0111: вхідний годинник PLL множиться на 9

    // Увімкнути PLL
    RCC->CR |= RCC_CR_PLLON;
    // Чекати, поки PLL стабілізується
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Очистити налаштування перемикання системного годинника
    RCC->CFGR &= ~RCC_CFGR_SW; // SW: перемикання системного годинника, 00: вибрано HSI як системний годинник

    // Вибрати PLL як джерело системного годинника
    RCC->CFGR |= RCC_CFGR_SW_PLL; // SW: перемикання системного годинника, 10: вибрано PLL як системний годинник

    // Чекати завершення перемикання на PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // Налаштування таймеру SysTick для 1 мс
    SysTick_Config(1000); // LL_Init1msTick(72000000);

    // Налаштування тактового сигналу для USB (72 МГц / 1.5 = 48 МГц)
    RCC->CFGR |= 0; // USB тактується правильно (48 МГц)

    // Налаштування такту АЦП (APB2 / 6 = 12 МГц)
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
}
