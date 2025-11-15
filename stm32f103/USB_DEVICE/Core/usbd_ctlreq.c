// usbd_ctlreq.c

// Обробка стандартних USB-запитів (Device, Interface, Endpoint) для STM32 USB пристрою

#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"

// Прототипи внутрішніх статичних функцій, що обробляють окремі типи запитів
static void USBD_GetDescriptor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_SetFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_ClrFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GetLen(uint8_t *buf);

/**
  * @brief  Обробка стандартних USB-запитів для пристрою
  *         Основна функція для обробки USB запитів типу Device (адресація, конфігурація, дескриптори і т.п.)
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval USBD_StatusTypeDef - результат обробки запиту
  */
USBD_StatusTypeDef USBD_StdDevReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_StatusTypeDef ret = USBD_OK;

  // Визначаємо тип запиту (клас, вендор, стандартний) за bmRequest полем
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR:
      // Запити класу або вендора передаємо у callback класу пристрою
      pdev->pClass->Setup(pdev, req);
      break;

    case USB_REQ_TYPE_STANDARD:
      // Обробка стандартних запитів
      switch (req->bRequest)
      {
        case USB_REQ_GET_DESCRIPTOR:  // Запит дескриптора
          USBD_GetDescriptor(pdev, req);
          break;

        case USB_REQ_SET_ADDRESS:     // Встановлення USB адреси
          USBD_SetAddress(pdev, req);
          break;

        case USB_REQ_SET_CONFIGURATION:  // Встановлення конфігурації пристрою
          USBD_SetConfig(pdev, req);
          break;

        case USB_REQ_GET_CONFIGURATION:  // Отримання поточної конфігурації
          USBD_GetConfig(pdev, req);
          break;

        case USB_REQ_GET_STATUS:  // Отримання статусу пристрою
          USBD_GetStatus(pdev, req);
          break;

        case USB_REQ_SET_FEATURE:  // Встановлення властивостей (наприклад, remote wakeup)
          USBD_SetFeature(pdev, req);
          break;

        case USB_REQ_CLEAR_FEATURE:  // Скидання властивостей
          USBD_ClrFeature(pdev, req);
          break;

        default:
          // Якщо запит не розпізнано - генеруємо помилку
          USBD_CtlError(pdev, req);
          break;
      }
      break;

    default:
      // Запити іншого типу - помилка
      USBD_CtlError(pdev, req);
      break;
  }

  return ret;
}

/**
  * @brief  Обробка стандартних USB-запитів для інтерфейсу
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval USBD_StatusTypeDef - статус обробки
  */
USBD_StatusTypeDef USBD_StdItfReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_StatusTypeDef ret = USBD_OK;

  // За типом запиту обробляємо та передаємо в клас для обробки інтерфейсних команд
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR:
    case USB_REQ_TYPE_STANDARD:
      switch (pdev->dev_state)
      {
        case USBD_STATE_DEFAULT:
        case USBD_STATE_ADDRESSED:
        case USBD_STATE_CONFIGURED:
          if (LOBYTE(req->wIndex) <= USBD_MAX_NUM_INTERFACES)
          {
            // Виклик хендлера класу для інтерфейсного запиту
            ret = (USBD_StatusTypeDef)pdev->pClass->Setup(pdev, req);
            if ((req->wLength == 0U) && (ret == USBD_OK))
            {
              // Якщо нема передачі даних - відправляємо статус OK
              USBD_CtlSendStatus(pdev);
            }
          }
          else
          {
            // Якщо вказано інтерфейс за межами максимальної кількості - помилка
            USBD_CtlError(pdev, req);
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
  }

  return USBD_OK;
}

/**
  * @brief  Обробка стандартних USB-запитів для кінцевої точки
  *         Підтримка запитів управління кінцевими точками (STALL, status, feature)
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval USBD_StatusTypeDef - статус обробки
  */
USBD_StatusTypeDef USBD_StdEPReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_EndpointTypeDef *pep;
  uint8_t   ep_addr;
  USBD_StatusTypeDef ret = USBD_OK;

  ep_addr  = LOBYTE(req->wIndex);  // Адреса ендпойнта (основною інформацією)

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR:
      // Передача класових/вендорних команд класу пристрою
      pdev->pClass->Setup(pdev, req);
      break;

    case USB_REQ_TYPE_STANDARD:
      // Перевірка типу запиту (класові запити всередині стандартних)
      if ((req->bmRequest & 0x60U) == 0x20U)
      {
        ret = (USBD_StatusTypeDef)pdev->pClass->Setup(pdev, req);
        return ret;
      }
      // Обробка стандартних основних запитів по кінцевих точках
      switch (req->bRequest)
      {
        case USB_REQ_SET_FEATURE:
          switch (pdev->dev_state)
          {
            case USBD_STATE_ADDRESSED:
              if ((ep_addr != 0x00U) && (ep_addr != 0x80U))
              {
                // Ставимо STALL на ендпойнти, якщо адресу розподілено, та запит адресації некоректний
                USBD_LL_StallEP(pdev, ep_addr);
                USBD_LL_StallEP(pdev, 0x80U);
              }
              else
              {
                USBD_CtlError(pdev, req);
              }
              break;

            case USBD_STATE_CONFIGURED:
              if (req->wValue == USB_FEATURE_EP_HALT)
              {
                if ((ep_addr != 0x00U) &&
                    (ep_addr != 0x80U) && (req->wLength == 0x00U))
                {
                  // Ставимо STALL на вказаний ендпойнт
                  USBD_LL_StallEP(pdev, ep_addr);
                }
              }
              USBD_CtlSendStatus(pdev);
              break;

            default:
              USBD_CtlError(pdev, req);
              break;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          switch (pdev->dev_state)
          {
            case USBD_STATE_ADDRESSED:
              if ((ep_addr != 0x00U) && (ep_addr != 0x80U))
              {
                // Помилка, якщо в стані ADDRESSED при очистці фіч
                USBD_LL_StallEP(pdev, ep_addr);
                USBD_LL_StallEP(pdev, 0x80U);
              }
              else
              {
                USBD_CtlError(pdev, req);
              }
              break;

            case USBD_STATE_CONFIGURED:
              if (req->wValue == USB_FEATURE_EP_HALT)
              {
                if ((ep_addr & 0x7FU) != 0x00U)
                {
                  // Знімаємо STALL з ендпойнта
                  USBD_LL_ClearStallEP(pdev, ep_addr);
                }
                USBD_CtlSendStatus(pdev);
              }
              break;

            default:
              USBD_CtlError(pdev, req);
              break;
          }
          break;

        case USB_REQ_GET_STATUS:
          switch (pdev->dev_state)
          {
            case USBD_STATE_ADDRESSED:
              if ((ep_addr != 0x00U) && (ep_addr != 0x80U))
              {
                USBD_CtlError(pdev, req);
                break;
              }
              // Визначаємо статус ендпойнта у буфер статусу
              pep = ((ep_addr & 0x80U) == 0x80U) ? &pdev->ep_in[ep_addr & 0x7FU] : &pdev->ep_out[ep_addr & 0x7FU];
              pep->status = 0x0000U;  // Стандартний статус - немає помилок або хальтів
              USBD_CtlSendData(pdev, (uint8_t *)(void *)&pep->status, 2U);
              break;

            case USBD_STATE_CONFIGURED:
              if ((ep_addr & 0x80U) == 0x80U)
              {
                if (pdev->ep_in[ep_addr & 0xFU].is_used == 0U)
                {
                  USBD_CtlError(pdev, req);
                  break;
                }
              }
              else
              {
                if (pdev->ep_out[ep_addr & 0xFU].is_used == 0U)
                {
                  USBD_CtlError(pdev, req);
                  break;
                }
              }

              pep = ((ep_addr & 0x80U) == 0x80U) ? &pdev->ep_in[ep_addr & 0x7FU] : &pdev->ep_out[ep_addr & 0x7FU];

              // Визначаємо статус ендпойнта: 0x0001 - STALL, 0x0000 - нормальна робота
              if ((ep_addr == 0x00U) || (ep_addr == 0x80U))
              {
                pep->status = 0x0000U;
              }
              else if (USBD_LL_IsStallEP(pdev, ep_addr))
              {
                pep->status = 0x0001U;
              }
              else
              {
                pep->status = 0x0000U;
              }
              USBD_CtlSendData(pdev, (uint8_t *)(void *)&pep->status, 2U);
              break;

            default:
              USBD_CtlError(pdev, req);
              break;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
  }

  return ret;
}

/**
  * @brief  Обробка запиту Get Descriptor - отримання дескрипторів пристрою
  * @details Обробляє всі типи дескрипторів: Device, Configuration, String, BOS (за потреби), Device Qualifier
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval void
  */
static void USBD_GetDescriptor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint8_t err = 0U;

  switch (req->wValue >> 8)  // Визначаємо тип дескриптора (старший байт wValue)
  {
#if (USBD_LPM_ENABLED == 1U)
    case USB_DESC_TYPE_BOS:
      if (pdev->pDesc->GetBOSDescriptor != NULL)
      {
        pbuf = pdev->pDesc->GetBOSDescriptor(pdev->dev_speed, &len);
      }
      else
      {
        USBD_CtlError(pdev, req);
        err++;
      }
      break;
#endif

    case USB_DESC_TYPE_DEVICE:
      pbuf = pdev->pDesc->GetDeviceDescriptor(pdev->dev_speed, &len);
      break;

    case USB_DESC_TYPE_CONFIGURATION:
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        pbuf = pdev->pClass->GetHSConfigDescriptor(&len);
        pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
      }
      else
      {
        pbuf = pdev->pClass->GetFSConfigDescriptor(&len);
        pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
      }
      break;

    case USB_DESC_TYPE_STRING:
      switch ((uint8_t)(req->wValue))
      {
        case USBD_IDX_LANGID_STR:
          if (pdev->pDesc->GetLangIDStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;

        case USBD_IDX_MFC_STR:
          if (pdev->pDesc->GetManufacturerStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;

        case USBD_IDX_PRODUCT_STR:
          if (pdev->pDesc->GetProductStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;

        case USBD_IDX_SERIAL_STR:
          if (pdev->pDesc->GetSerialStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetSerialStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;

        case USBD_IDX_CONFIG_STR:
          if (pdev->pDesc->GetConfigurationStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetConfigurationStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;

        case USBD_IDX_INTERFACE_STR:
          if (pdev->pDesc->GetInterfaceStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetInterfaceStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;

        default:
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
          if (pdev->pClass->GetUsrStrDescriptor != NULL)
          {
            pbuf = pdev->pClass->GetUsrStrDescriptor(pdev, (req->wValue), &len);
          }
          else
          {
            USBD_CtlError(pdev, req);
            err++;
          }
          break;
#else
          USBD_CtlError(pdev, req);
          err++;
#endif
      }
      break;

    case USB_DESC_TYPE_DEVICE_QUALIFIER:
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        pbuf = pdev->pClass->GetDeviceQualifierDescriptor(&len);
      }
      else
      {
        USBD_CtlError(pdev, req);
        err++;
      }
      break;

    case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        pbuf = pdev->pClass->GetOtherSpeedConfigDescriptor(&len);
        pbuf[1] = USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION;
      }
      else
      {
        USBD_CtlError(pdev, req);
        err++;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      err++;
      break;
  }

  if (err != 0U)
  {
    return; // Якщо сталася помилка - вихід
  }
  else
  {
    if ((len != 0U) && (req->wLength != 0U))
    {
      // Відправляємо дескриптор, обмежений довжиною запиту
      len = MIN(len, req->wLength);
      (void)USBD_CtlSendData(pdev, pbuf, len);
    }
    if (req->wLength == 0U)
    {
      // Якщо дані не потрібні - відправляємо статус ОК
      (void)USBD_CtlSendStatus(pdev);
    }
  }
}

/**
  * @brief  Обробка запиту встановлення USB адреси пристрою
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту з адресою
  * @retval void
  */
static void USBD_SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t  dev_addr;

  // Переконуємось, що запит коректний (адреса < 128, індекс і довжина 0)
  if ((req->wIndex == 0U) && (req->wLength == 0U) && (req->wValue < 128U))
  {
    dev_addr = (uint8_t)(req->wValue) & 0x7FU;

    // Заборонено змінювати адресу, якщо пристрій у конфігурованому стані
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
      USBD_CtlError(pdev, req);
    }
    else
    {
      // Встановлюємо адресу пристрою і оновлюємо стан
      pdev->dev_address = dev_addr;
      USBD_LL_SetUSBAddress(pdev, dev_addr);
      USBD_CtlSendStatus(pdev);

      if (dev_addr != 0U)
      {
        pdev->dev_state = USBD_STATE_ADDRESSED;
      }
      else
      {
        pdev->dev_state = USBD_STATE_DEFAULT;
      }
    }
  }
  else
  {
    USBD_CtlError(pdev, req);
  }
}

/**
  * @brief  Обробка запиту встановлення конфігурації пристрою
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту з номером конфігурації
  * @retval void
  */
static void USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  static uint8_t cfgidx;

  cfgidx = (uint8_t)(req->wValue);

  if (cfgidx > USBD_MAX_NUM_CONFIGURATION)
  {
    USBD_CtlError(pdev, req);
  }
  else
  {
    switch (pdev->dev_state)
    {
      case USBD_STATE_ADDRESSED:
        if (cfgidx)
        {
          pdev->dev_config = cfgidx;
          pdev->dev_state = USBD_STATE_CONFIGURED;

          if (USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL)
          {
            USBD_CtlError(pdev, req);
            return;
          }
          USBD_CtlSendStatus(pdev);
        }
        else
        {
          USBD_CtlSendStatus(pdev);
        }
        break;

      case USBD_STATE_CONFIGURED:
        if (cfgidx == 0U)
        {
          pdev->dev_state = USBD_STATE_ADDRESSED;
          pdev->dev_config = cfgidx;
          USBD_ClrClassConfig(pdev, cfgidx);
          USBD_CtlSendStatus(pdev);
        }
        else if (cfgidx != pdev->dev_config)
        {
          // Очищуємо стару та встановлюємо нову конфігурацію
          USBD_ClrClassConfig(pdev, (uint8_t)pdev->dev_config);
          pdev->dev_config = cfgidx;

          if (USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL)
          {
            USBD_CtlError(pdev, req);
            return;
          }
          USBD_CtlSendStatus(pdev);
        }
        else
        {
          USBD_CtlSendStatus(pdev);
        }
        break;

      default:
        USBD_CtlError(pdev, req);
        USBD_ClrClassConfig(pdev, cfgidx);
        break;
    }
  }
}

/**
  * @brief  Обробка запиту отримання поточної конфігурації пристрою
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval void
  */
static void USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  if (req->wLength != 1U)
  {
    USBD_CtlError(pdev, req);
  }
  else
  {
    switch (pdev->dev_state)
    {
      case USBD_STATE_DEFAULT:
      case USBD_STATE_ADDRESSED:
        pdev->dev_default_config = 0U;
        USBD_CtlSendData(pdev, (uint8_t *)(void *)&pdev->dev_default_config, 1U);
        break;

      case USBD_STATE_CONFIGURED:
        USBD_CtlSendData(pdev, (uint8_t *)(void *)&pdev->dev_config, 1U);
        break;

      default:
        USBD_CtlError(pdev, req);
        break;
    }
  }
}

/**
  * @brief  Обробка запиту отримання статусу USB пристрою
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval void
  */
static void USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  switch (pdev->dev_state)
  {
    case USBD_STATE_DEFAULT:
    case USBD_STATE_ADDRESSED:
    case USBD_STATE_CONFIGURED:
      if (req->wLength != 0x2U)
      {
        USBD_CtlError(pdev, req);
        break;
      }

#if (USBD_SELF_POWERED == 1U)
      pdev->dev_config_status = USB_CONFIG_SELF_POWERED;
#else
      pdev->dev_config_status = 0U;
#endif

      if (pdev->dev_remote_wakeup)
      {
        pdev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;
      }

      // Відправляємо 2-байтовий статус пристрою
      USBD_CtlSendData(pdev, (uint8_t *)(void *)&pdev->dev_config_status, 2U);
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
  }
}

/**
  * @brief  Обробка встановлення функції пристрою (feature)
  *         Наприклад, увімкнення Remote Wakeup
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval void
  */
static void USBD_SetFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
  {
    pdev->dev_remote_wakeup = 1U;
    USBD_CtlSendStatus(pdev);
  }
}

/**
  * @brief  Обробка очищення встановленої функції пристрою (feature)
  *         Наприклад, вимкнення Remote Wakeup
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту
  * @retval void
  */
static void USBD_ClrFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  switch (pdev->dev_state)
  {
    case USBD_STATE_DEFAULT:
    case USBD_STATE_ADDRESSED:
    case USBD_STATE_CONFIGURED:
      if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
      {
        pdev->dev_remote_wakeup = 0U;
        USBD_CtlSendStatus(pdev);
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
  }
}

/**
  * @brief  Копіює область пам'яті з прийнятим setup-запитом в структуру USBD_SetupReqTypedef
  * @param  req: вказівник на структуру для збереження запиту
  * @param  pdata: буфер з отриманими 8 байтами USB setup пакету
  * @retval void
  */
void USBD_ParseSetupRequest(USBD_SetupReqTypedef *req, uint8_t *pdata)
{
  req->bmRequest = *(uint8_t *)(pdata);
  req->bRequest = *(uint8_t *)(pdata + 1U);
  req->wValue = SWAPBYTE(pdata + 2U);
  req->wIndex = SWAPBYTE(pdata + 4U);
  req->wLength = SWAPBYTE(pdata + 6U);
}

/**
  * @brief  Обробка помилок USB-запитів
  *         Встановлює STALL на контрольних ендпойнтах (IN та OUT)
  * @param  pdev: дескриптор структури USB пристрою
  * @param  req: дескриптор USB setup-запиту, який викликав помилку
  * @retval void
  */
void USBD_CtlError(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_LL_StallEP(pdev, 0x80U);
  USBD_LL_StallEP(pdev, 0x00U);
}

/**
  * @brief  Конвертує ASCII рядок у Unicode формат, що використовується у USB дескрипторах рядків
  *         Формат: [довжина][тип дескриптора][Unicode символи...]
  * @param  desc: вхідний ASCII рядок
  * @param  unicode: буфер для збереження Unicode рядка
  * @param  len: вказівник на змінну довжини, у яку записується довжина дескриптора
  * @retval void
  */
void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
  uint8_t idx = 0U;

  if (desc != NULL)
  {
    // Довжина - кількість символів * 2 (unicode) + 2 байти на заголовок
    *len = (uint16_t)USBD_GetLen(desc) * 2U + 2U;

    unicode[idx++] = *(uint8_t *)(void *)len;  // Довжина дескриптора
    unicode[idx++] = USB_DESC_TYPE_STRING;     // Тип дескриптора - рядок USB

    // Конвертація ASCII символів у unicode (другий байт нульовий)
    while (*desc != '\0')
    {
      unicode[idx++] = *desc++;
      unicode[idx++] = 0U;
    }
  }
}

/**
  * @brief  Обчислення довжини ASCII рядка
  * @param  buf: вхідний ASCII рядок
  * @retval uint8_t - довжина рядка
  */
static uint8_t USBD_GetLen(uint8_t *buf)
{
  uint8_t  len = 0U;
  while (*buf != '\0')
  {
    len++;
    buf++;
  }
  return len;
}

