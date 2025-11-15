// main.c

#include "main.h"
#include "usb_device.h"

#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "adc_read.h"
#include "gpio_init.h"
#include "SystemClock_Config.h"
#include "generate_test_signals.h"

#define HISTORY_SIZE 500
#define CHANNELS_TO_SEND 2 // Наприклад, канал 2 та 3
#define PACKET_SIZE 13 // 1 байт старт + 4 канали * 3 байти (ID + 2 байти даних)

extern USBD_DescriptorsTypeDef FS_Desc;
extern USBD_ClassTypeDef  USBD_CDC;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint16_t new_rate;
extern uint16_t test_signal;

// void LL_mDelay(uint32_t Delay);
void SystemClock_Config(void);

// Функція налаштування піну для USB (для скидування USB)
void USB_GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // PA11 (DM) - input floating
    GPIOA->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOA->CRH |= GPIO_CRH_CNF11_0;

    // PA12 (DP) - AF push-pull output 50MHz
    GPIOA->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12);
    GPIOA->CRH |= GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0;   // Output mode 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF12_1;                        // Alternate function push-pull

    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
}

// Функція налаштування піну для USB (для скидування USB)
void USB_DEVICE_PinReset(void)
{
    // Увімкнення тактуючого сигналу для GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Налаштування піну PA12 як вихід (Push-Pull, високошвидкісний)
    GPIOA->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12); // Очищаємо біти
    GPIOA->CRH |= (GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0); // Output mode, 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF12_0; // General purpose output push-pull

    // Встановлення логічного рівня високого на PA12
    GPIOA->BSRR = GPIO_BSRR_BS12;

    // Невелика затримка (імітація)
    for (volatile int i = 0; i < 720000; i++); // ~10 мс (на 72 МГц)

    // Встановлення логічного рівня низького на PA12
    GPIOA->BSRR = GPIO_BSRR_BR12;
}

void USB_DEVICE_Init(void)
{
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
  USBD_Start(&hUsbDeviceFS);
}

void init_osc_data(OscData *oscData) {
    static float channel_a_history[HISTORY_SIZE] = {0};
    static float channel_b_history[HISTORY_SIZE] = {0};
    static float channel_c_history[HISTORY_SIZE] = {0};
    static float channel_d_history[HISTORY_SIZE] = {0};

    float *channel_history[MAX_CHANNELS]; // Ініціалізуйте і виділіть пам'ять десь у init

    oscData->channel_history[0] = channel_a_history;
    oscData->channel_history[1] = channel_b_history;
    oscData->channel_history[2] = channel_c_history;
    oscData->channel_history[3] = channel_d_history;
}

int main(void)
{
  /* USER CODE BEGIN 1 */

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN); // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN); // LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(0x3U);
  MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE); // LL_GPIO_AF_Remap_SWJ_NOJTAG();

  SystemClock_Config();

  int SystemCoreClock = 72000000; // 72 MHz
  // Кількість тактів за 1 мс = SystemCoreClock / 1000.
  // Викличте SysTick_Config з обчисленим значенням
  SysTick_Config(SystemCoreClock / 1000); // 1мсек, для LL_mDelay

  gpio_init(GPIOC, 13, GPIO_MODE_OUTPUT_PP);

  USB_GPIO_Init();
  USB_DEVICE_PinReset();

  USB_DEVICE_Init();
  USBD_LL_SetSpeed(&hUsbDeviceFS, USBD_SPEED_FULL);
  USBD_LL_Reset(&hUsbDeviceFS);

  LL_mDelay(100);

  float *channel_history[MAX_CHANNELS]; // Ініціалізуйте і виділіть пам'ять десь у init
  OscData oscData = { .channel_history = {channel_history[0], channel_history[1], channel_history[2], channel_history[3]} };
  uint16_t usb_send_buf[CHANNELS_TO_SEND * HISTORY_SIZE];

  LL_mDelay(100);

  init_osc_data(&oscData);

  // Ініціалізуємо піни для PA0, PA1, PA2, PA3
  Init_ADC_Pin(GPIOA, 0); // Налаштувати PA0 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 1); // Налаштувати PA1 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 2); // Налаштувати PA2 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 3); // Налаштувати PA3 як аналоговий вхід

  // Ініціалізація ADC
  Init_ADC(ADC1);

  // Читання значень з каналів PA0, PA1, PA2, PA3
  uint16_t value_PA0; // PA0
  uint16_t value_PA1; // PA1
  uint16_t value_PA2; // PA2
  uint16_t value_PA3; // PA3

  uint8_t Temp_Buffer[APP_RX_DATA_SIZE] = { 0 };

  // generate_test_signals4(&oscData, 500, 0.0f);
  // generate_test_signals(&oscData, 500, 0.0f);
  // generate_gaussian_envelope_signal(&oscData, 500, 0.0f);
  generate_test_signals_extended(&oscData, 500, 0.0f);

  static uint16_t history_index = 0;

  while (1)
  {
      uint8_t usb_send_buf[PACKET_SIZE];
      usb_send_buf[0] = 0xAA; // Стартовий байт

      if (test_signal)
      {
          // Відправляємо дані з генератора тестових сигналів
          for (int ch = 0; ch < 4; ch++)
          {
              int16_t val = (int16_t)oscData.channel_history[ch][history_index];
              usb_send_buf[1 + ch * 3] = ch; // ID каналу
              usb_send_buf[1 + ch * 3 + 1] = val & 0xFF; // Молодший байт
              usb_send_buf[1 + ch * 3 + 2] = (val >> 8); // Старший байт
          }
          history_index++;
          if (history_index >= HISTORY_SIZE)
              history_index = 0;
      }
      else
      {
          // Зчитуємо актуальні значення з АЦП
          // Читання значень з каналів PA0, PA1, PA2, PA3
          /* Інтерпретуємо дані як негативні якщо вони нижчі за 2048 (умовний нуль)
          для центування даних на осцилоскопі */
          value_PA0 = Read_ADC(ADC1,0) - 2048; // PA0
          value_PA1 = Read_ADC(ADC1,1) - 2048; // PA1
          value_PA2 = Read_ADC(ADC1,2) - 2048; // PA2
          value_PA3 = Read_ADC(ADC1,3) - 2048; // PA2

          int16_t adc_values[4] = {value_PA0, value_PA1, value_PA2, value_PA3};

          for (int ch = 0; ch < 4; ch++)
          {
              usb_send_buf[1 + ch * 3] = ch; // ID каналу
              usb_send_buf[1 + ch * 3 + 1] = adc_values[ch] & 0xFF; // Молодший байт
              usb_send_buf[1 + ch * 3 + 2] = (adc_values[ch] >> 8); // Старший байт
          }
      }

      CDC_Transmit_FS(usb_send_buf, PACKET_SIZE);

      // Затримка або інтервал між відправками (можна замінити на таймер)
      for (volatile uint32_t delay = 0; delay < new_rate * 1000; delay++)
          __asm volatile ("nop");

      // Індикатори стану (за вашим кодом)
      if (new_rate % 10)
          gpio_write_pin(GPIOC, 13, 0);
      else
          gpio_write_pin(GPIOC, 13, 1);

      if (test_signal)
          gpio_toggle_pin(GPIOC, 13);
  }

}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */

