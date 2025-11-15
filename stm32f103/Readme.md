## Збірка прошивки для STM32F103 (Bluepill)

Перейдіть у директорію `stm32f103-ll-adc-print` та виконайте:

make

Це створить файл `build/app/application.elf`, який можна прошити у мікроконтролер.

### Додаткові цілі

- `make clean` — видаляє всі проміжні та фінальні файли збірки (`*.o`, `*.elf`, `*.bin`).

### Вимоги

- Встановлений [arm-none-eabi-gcc](https://developer.arm.com/downloads/-/gnu-rm) (компілятор для ARM Cortex-M)
- Файли `startup_stm32f103xb.s` та `STM32F103C8Tx_FLASH.ld` мають бути у цій директорії

### Прошивка

Файл `build/app/application.elf` можна прошити у мікроконтролер за допомогою програматора (наприклад, st-link, j-link, або USB-UART bootloader).
