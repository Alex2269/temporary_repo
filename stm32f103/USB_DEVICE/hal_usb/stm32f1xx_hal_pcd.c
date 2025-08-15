// stm32f1xx_hal_pcd.c

// #include "usbd_core.h"
#include "usbd_cdc.h"

// Попереднє оголошення допоміжних функцій для обробки ендпоінт.
static USBD_StatusTypeDef USBD_PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd);
static USBD_StatusTypeDef Handle_EP0(PCD_HandleTypeDef *hpcd, uint16_t wIstr);
static USBD_StatusTypeDef Handle_EP0_IN(PCD_HandleTypeDef *hpcd);
static USBD_StatusTypeDef Handle_EP0_OUT_SETUP(PCD_HandleTypeDef *hpcd, uint16_t wIstr);
static USBD_StatusTypeDef Handle_NonControl_EP(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wIstr);
static USBD_StatusTypeDef Handle_OUT_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal);
static USBD_StatusTypeDef Handle_IN_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal);

// ================= callback begin =========================

void USBD_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

void USBD_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void USBD_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void USBD_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

void USBD_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
  if ( hpcd->Init.speed != USBD_PCD_SPEED_FULL)
  {
    Error_Handler();
  }
  USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);
  USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

void USBD_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
  if (hpcd->Init.low_power_enable)
  {
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
}

void USBD_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

void USBD_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void USBD_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void USBD_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

void USBD_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}
// ================= callback end ===========================

USBD_StatusTypeDef  USBD_PCDEx_PMAConfig(PCD_HandleTypeDef *hpcd, uint16_t ep_addr, uint16_t ep_kind, uint32_t pmaadress)
{
  USBD_PCD_EPTypeDef *ep;

  /* initialize ep structure*/
  if ((0x80U & ep_addr) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr];
  }

  /* Here we check if the endpoint is single or double Buffer*/
  if (ep_kind == USBD_PCD_SNG_BUF)
  {
    /* Single Buffer */
    ep->doublebuffer = 0U;
    /* Configure the PMA */
    ep->pmaadress = (uint16_t)pmaadress;
  }
  return USBD_OK;
}

// Основна функція переривань контролера PCD (USB)
void USBD_PCD_IRQHandler(PCD_HandleTypeDef *hpcd)
{
  uint32_t wIstr = USB_ReadInterrupts(hpcd->Instance); // Зчитуємо всі переривання

  // Обробка переривання контролю передачі (CTR)
  if ((wIstr & USB_ISTR_CTR) == USB_ISTR_CTR)
  {
    (void)USBD_PCD_EP_ISR_Handler(hpcd); // Обробка ендпоінти через ISR (Interrupt Service Routine)
    return;
  }

  // Обробка переривання Reset
  if ((wIstr & USB_ISTR_RESET) == USB_ISTR_RESET)
  {
    // Очищаємо флаг Reset
    CLEAR_BIT(hpcd->Instance->ISTR, USB_ISTR_RESET);

    USBD_PCD_ResetCallback(hpcd); // Викликаємо callback-функцію при Reset
    (void)USBD_PCD_SetAddress(hpcd, 0U); // Встановлюємо адресу пристрою на 0
  }

  // Очищення інших флагів переривань
  CLEAR_BIT(hpcd->Instance->ISTR, USB_ISTR_PMAOVR | USB_ISTR_ERR | USB_ISTR_WKUP | USB_ISTR_RESET | USB_ISTR_SUSP | USB_ISTR_SOF | USB_ISTR_ESOF);
}

// Функція для встановлення адреси USB пристрою
USBD_StatusTypeDef USBD_PCD_SetAddress(PCD_HandleTypeDef *hpcd, uint8_t address)
{
  hpcd->USB_Address = address; // Задаємо адресу
  (void)USB_SetDevAddress(hpcd->Instance, address); // Налаштовуємо адресу на контролері
  return USBD_OK;
}

// Ініціалізація PCD контролера
USBD_StatusTypeDef USBD_PCD_Init(PCD_HandleTypeDef *hpcd)
{
  USBD_PCD_MspInit(hpcd); // Ініціалізація периферії (GPIO, DMA, etc.)
  (void)USB_DisableGlobalInt(hpcd->Instance); // Вимикаємо PCD контролер

  // Ініціалізація ендпоінт
  for (uint8_t i = 0U; i < hpcd->Init.dev_endpoints; i++)
  {
    hpcd->IN_ep[i] = (USBD_PCD_EPTypeDef){.is_in = 1U, .num = i, .type = EP_TYPE_CTRL};
    hpcd->OUT_ep[i] = (USBD_PCD_EPTypeDef){.is_in = 0U, .num = i, .type = EP_TYPE_CTRL};
  }

  hpcd->USB_Address = 0U;
  hpcd->State = USBD_PCD_STATE_READY; // Переходимо в стан "готовий"

  (void)USB_DevDisconnect(hpcd->Instance); // Вимикаємо пристрій з USB
  return USBD_OK;
}

// Функція для запуску PCD контролера
USBD_StatusTypeDef USBD_PCD_Start(PCD_HandleTypeDef *hpcd)
{
  (void)USB_EnableGlobalInt(hpcd->Instance); // Включаємо PCD контролер
  (void)USB_DevConnect(hpcd->Instance); // Підключаємо пристрій до USB
  return USBD_OK;
}

// Функція для відкриття ендпоінти
USBD_StatusTypeDef USBD_PCD_EP_Open(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint16_t ep_mps, uint8_t ep_type)
{
  // Операції налаштування ендпоінти
  USBD_PCD_EPTypeDef *ep = (ep_addr & 0x80U) ? &hpcd->IN_ep[ep_addr & EP_ADDR_MSK] : &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];

  ep->is_in = (ep_addr & 0x80U) != 0U; // Визначаємо напрямок передачі
  ep->num = ep_addr & EP_ADDR_MSK;
  ep->maxpacket = ep_mps;
  ep->type = ep_type;

  (void)USB_ActivateEndpoint(hpcd->Instance, ep); // Активуємо ендпоінт
  return USBD_OK;
}

// Функція для закриття ендпоінти
USBD_StatusTypeDef USBD_PCD_EP_Close(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  // Операції закриття ендпоінти
  USBD_PCD_EPTypeDef *ep = (ep_addr & 0x80U) ? &hpcd->IN_ep[ep_addr & EP_ADDR_MSK] : &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];

  ep->is_in = (ep_addr & 0x80U) != 0U;
  ep->num = ep_addr & EP_ADDR_MSK;

  (void)USB_DeactivateEndpoint(hpcd->Instance, ep); // Деактивуємо ендпоінт
  return USBD_OK;
}

// Функція для встановлення Stall (заставки) на ендпоінт
USBD_StatusTypeDef USBD_PCD_EP_SetStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  // Встановлення заставки на ендпоінт
  USBD_PCD_EPTypeDef *ep = (ep_addr & 0x80U) ? &hpcd->IN_ep[ep_addr & EP_ADDR_MSK] : &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];

  ep->is_in = (ep_addr & 0x80U) != 0U;
  ep->is_stall = 1U; // Встановлюємо флаг для Stall
  ep->num = ep_addr & EP_ADDR_MSK;

  (void)USB_EPSetStall(hpcd->Instance, ep); // Дійсно встановлюємо Stall

  if (ep_addr == 0U)
    (void)USB_EP0_OutStart(hpcd->Instance, (uint8_t *)hpcd->Setup);

  return USBD_OK;
}

// Функція для передачі даних через ендпоінт (IN ендпоінти)
USBD_StatusTypeDef USBD_PCD_EP_Transmit(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];

  ep->xfer_buff = pBuf;
  ep->xfer_len = len;
  ep->xfer_count = 0U;
  ep->is_in = 1U;
  ep->num = ep_addr & EP_ADDR_MSK;

  (void)USB_EPStartXfer(hpcd->Instance, ep); // Починаємо передачу даних
  return USBD_OK;
}

// Функція для прийому даних через ендпоінт (OUT ендпоінти)
USBD_StatusTypeDef USBD_PCD_EP_Receive(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];

  ep->xfer_buff = pBuf;
  ep->xfer_len = len;
  ep->xfer_count = 0U;
  ep->is_in = 0U;
  ep->num = ep_addr & EP_ADDR_MSK;

  (void)USB_EPStartXfer(hpcd->Instance, ep); // Починаємо прийом даних
  return USBD_OK;
}

// Отримання кількості прийнятих байт на OUT ендпоінт
uint32_t USBD_PCD_EP_GetRxCount(PCD_HandleTypeDef const *hpcd, uint8_t ep_addr)
{
  return hpcd->OUT_ep[ep_addr & EP_ADDR_MSK].xfer_count; // Повертаємо кількість прийнятих байт
}

// Обробник переривань для ендпоінтів (EP)
static USBD_StatusTypeDef USBD_PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd)
{
    uint16_t wIstr = hpcd->Instance->ISTR; // Читання значення переривання з реєстру ISTR
    uint8_t epindex = (uint8_t)(wIstr & USB_ISTR_EP_ID); // Отримуємо індекс ендпоінта, використовуючи бітову маску

    if ((wIstr & USB_ISTR_CTR) == 0U) // Перевіряємо, чи є переривання за керуванням (CTR)
        return USBD_OK;

    // Обробка для контрольного ендпоінта (EP0)
    if (epindex == 0U)
    {
        return Handle_EP0(hpcd, wIstr); // Якщо переривання на EP0, викликаємо обробник для EP0
    }
    // Обробка для неконтрольних ендпоінтів
    else
    {
        return Handle_NonControl_EP(hpcd, epindex, wIstr); // Для інших ендпоінтів викликаємо обробник
    }
}

// Обробник для контрольного ендпоінта (EP0)
static USBD_StatusTypeDef Handle_EP0(PCD_HandleTypeDef *hpcd, uint16_t wIstr)
{
    USBD_PCD_EPTypeDef *ep; // Структура для ендпоінта

    if ((wIstr & USB_ISTR_DIR) == 0U) // Якщо напрямок передачі IN
    {
        Handle_EP0_IN(hpcd); // Обробляємо IN-передачу для EP0
    }
    else // Якщо напрямок OUT або SETUP
    {
        Handle_EP0_OUT_SETUP(hpcd, wIstr); // Обробляємо OUT або SETUP-передачу для EP0
    }

    return USBD_OK;
}

// Обробник для IN-передачі для EP0
static USBD_StatusTypeDef Handle_EP0_IN(PCD_HandleTypeDef *hpcd)
{
    USBD_PCD_EPTypeDef *ep = &hpcd->IN_ep[0]; // Отримуємо ендпоінт IN для EP0

    USBD_PCD_CLEAR_TX_EP_CTR(hpcd->Instance, USBD_PCD_ENDP0); // Очищаємо контрольний біт передачі для EP0
    ep->xfer_count = USBD_PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num); // Отримуємо кількість переданих байтів
    ep->xfer_buff += ep->xfer_count; // Оновлюємо буфер передачі

    USBD_PCD_DataInStageCallback(hpcd, 0U); // Викликаємо колбек для завершення стадії передачі даних IN

    if ((hpcd->USB_Address > 0U) && (ep->xfer_len == 0U)) // Якщо адреса пристрою встановлена і довжина передачі 0
    {
        hpcd->Instance->DADDR = ((uint16_t)hpcd->USB_Address | USB_DADDR_EF); // Встановлюємо адресу пристрою
        hpcd->USB_Address = 0U; // Скидаємо адресу
    }

    return USBD_OK;
}

// Обробник для OUT/SETUP передачі для EP0
static USBD_StatusTypeDef Handle_EP0_OUT_SETUP(PCD_HandleTypeDef *hpcd, uint16_t wIstr)
{
    USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[0]; // Отримуємо OUT ендпоінт для EP0
    uint16_t wEPVal = USBD_PCD_GET_ENDPOINT(hpcd->Instance, USBD_PCD_ENDP0); // Читання значення з реєстру ендпоінта

    if ((wEPVal & USB_EP_SETUP) != 0U) // Якщо це SETUP пакет
    {
        ep->xfer_count = USBD_PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num); // Отримуємо кількість отриманих байтів
        USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, (uint16_t)ep->xfer_count); // Читаємо дані в пам'ять
        USBD_PCD_CLEAR_RX_EP_CTR(hpcd->Instance, USBD_PCD_ENDP0); // Очищаємо контроль передачі для EP0
        USBD_PCD_SetupStageCallback(hpcd); // Викликаємо колбек для стадії SETUP
    }
    else if ((wEPVal & USB_EP_CTR_RX) != 0U) // Якщо прийшли дані в ендпоінт
    {
        USBD_PCD_CLEAR_RX_EP_CTR(hpcd->Instance, USBD_PCD_ENDP0); // Очищаємо біт контролю прийому
        ep->xfer_count = USBD_PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num); // Отримуємо кількість прийнятих байтів

        if (ep->xfer_count != 0U && ep->xfer_buff != 0U) // Якщо є дані для обробки
        {
            USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, (uint16_t)ep->xfer_count); // Читаємо дані з пам'яті
            ep->xfer_buff += ep->xfer_count; // Оновлюємо буфер
            USBD_PCD_DataOutStageCallback(hpcd, 0U); // Викликаємо колбек для передачі даних OUT
        }

        // Перевіряємо чи потрібно встановити кількість байтів для наступної передачі
        wEPVal = (uint16_t)USBD_PCD_GET_ENDPOINT(hpcd->Instance, USBD_PCD_ENDP0);
        if ((wEPVal & USB_EP_SETUP) == 0U && (wEPVal & USB_EP_RX_STRX) != USB_EP_RX_VALID)
        {
            USBD_PCD_SET_EP_RX_CNT(hpcd->Instance, USBD_PCD_ENDP0, ep->maxpacket); // Встановлюємо максимальний розмір передачі
        }
    }

    return USBD_OK;
}

// Обробник для неконтрольних ендпоінтів
static USBD_StatusTypeDef Handle_NonControl_EP(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wIstr)
{
    USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[epindex]; // Отримуємо OUT ендпоінт за індексом
    uint16_t wEPVal = USBD_PCD_GET_ENDPOINT(hpcd->Instance, epindex); // Читаємо значення ендпоінта

    // Обробка для OUT передачі
    if ((wEPVal & USB_EP_CTR_RX) != 0U)
    {
        Handle_OUT_Transfer(hpcd, epindex, wEPVal); // Якщо є дані на OUT ендпоінті, обробляємо передачу
    }

    // Обробка для IN передачі
    if ((wEPVal & USB_EP_CTR_TX) != 0U)
    {
        Handle_IN_Transfer(hpcd, epindex, wEPVal); // Якщо є дані на IN ендпоінті, обробляємо передачу
    }

    return USBD_OK;
}

// Обробка OUT передачі для неконтрольного ендпоінта
static USBD_StatusTypeDef Handle_OUT_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal)
{
    USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[epindex]; // Отримуємо OUT ендпоінт за індексом
    uint16_t count = USBD_PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num); // Отримуємо кількість прийнятих байтів

    USBD_PCD_CLEAR_RX_EP_CTR(hpcd->Instance, epindex); // Очищаємо контрольний біт прийому

    if (count != 0U) // Якщо є дані для обробки
    {
        USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, count); // Читаємо дані
    }

    ep->xfer_count += count; // Оновлюємо кількість переданих байтів
    ep->xfer_buff += count; // Оновлюємо буфер

    // Якщо передача завершена або отримано менше даних, ніж максимальний розмір
    if (ep->xfer_len == 0U || count < ep->maxpacket)
    {
        USBD_PCD_DataOutStageCallback(hpcd, ep->num); // Викликаємо колбек для завершення передачі
    }
    else
    {
        (void)USB_EPStartXfer(hpcd->Instance, ep); // Якщо передача не завершена, запускаємо нову передачу
    }

    return USBD_OK;
}

// Обробка IN передачі для неконтрольного ендпоінта
static USBD_StatusTypeDef Handle_IN_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal)
{
    USBD_PCD_EPTypeDef *ep = &hpcd->IN_ep[epindex]; // Отримуємо IN ендпоінт за індексом

    USBD_PCD_CLEAR_TX_EP_CTR(hpcd->Instance, epindex); // Очищаємо контрольний біт передачі

    if (ep->type == EP_TYPE_ISOC) // Якщо ендпоінт є ізохронним
    {
        ep->xfer_len = 0U; // Оновлюємо довжину передачі
        USBD_PCD_DataInStageCallback(hpcd, ep->num); // Викликаємо колбек для завершення передачі
    }
    else
    {
        if ((wEPVal & USB_EP_KIND) == 0U) // Якщо це багатопакетна передача
        {
            uint16_t TxPctSize = USBD_PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num); // Отримуємо кількість переданих байтів
            if (ep->xfer_len > TxPctSize)
            {
                ep->xfer_len -= TxPctSize; // Якщо є залишок, зменшуємо довжину передачі
            }
            else
            {
                ep->xfer_len = 0U; // Якщо передача завершена
            }

            if (ep->xfer_len == 0U)
            {
                USBD_PCD_DataInStageCallback(hpcd, ep->num); // Викликаємо колбек для завершення передачі
            }
            else
            {
                ep->xfer_buff += TxPctSize; // Оновлюємо буфер
                ep->xfer_count += TxPctSize; // Оновлюємо кількість переданих байтів
                (void)USB_EPStartXfer(hpcd->Instance, ep); // Якщо передача не завершена, запускаємо нову передачу
            }
        }
    }

    return USBD_OK;
}

// Функція для очищення стеллу на ендпоінті
USBD_StatusTypeDef USBD_PCD_EP_ClrStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
    return USBD_OK; // Повертаємо OK (не реалізовано)
}
