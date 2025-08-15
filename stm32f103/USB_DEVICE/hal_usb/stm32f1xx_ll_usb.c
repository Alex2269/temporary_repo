// stm32f1xx_ll_usb.c

#include "stm32f1xx_hal_pcd.h"

#define PMA_ACCESS 2U // Коефіцієнт доступу до памʼяті PMA

/** Вимикає глобальні USB-переривання */
USBD_StatusTypeDef USB_DisableGlobalInt(USB_TypeDef *USBx)
{
  uint32_t interrupt_mask = USB_CNTR_CTRM | USB_CNTR_WKUPM |
                            USB_CNTR_SUSPM | USB_CNTR_ERRM |
                            USB_CNTR_SOFM | USB_CNTR_ESOFM |
                            USB_CNTR_RESETM;
  USBx->CNTR &= (uint16_t)(~interrupt_mask);
  return USBD_OK;
}

/** Заглушка ініціалізації USB ядра (не використовується у FS) */
USBD_StatusTypeDef USB_CoreInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg)
{
  (void)USBx;
  (void)cfg;
  return USBD_OK;
}

/** Заглушка для встановлення режиму USB (неактуально для FS) */
USBD_StatusTypeDef USB_SetCurrentMode(USB_TypeDef *USBx, USB_ModeTypeDef mode)
{
  (void)USBx;
  (void)mode;
  return USBD_OK;
}

/** Ініціалізація USB пристрою */
USBD_StatusTypeDef USB_DevInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg)
{
  (void )cfg ;
  USBx ->CNTR = USB_CNTR_FRES ; // Скидання USB
  USBx->CNTR &= ~USB_CNTR_FRES; // Завершення скидання
  USBx->ISTR = 0; // Очищення прапорців
  USBx->DADDR = USB_DADDR_EF; // Увімкнення пристрою
  USBx->BTABLE = BTABLE_ADDRESS;
  return USBD_OK;
}

/** Увімкнення глобальних переривань USB */
USBD_StatusTypeDef USB_EnableGlobalInt(USB_TypeDef * USBx)
{
  USBx->ISTR = 0U;
  uint32_t interrupt_mask = USB_CNTR_CTRM | USB_CNTR_WKUPM |
                            USB_CNTR_SUSPM | USB_CNTR_ERRM |
                            USB_CNTR_SOFM | USB_CNTR_ESOFM |
                            USB_CNTR_RESETM;
  USBx->CNTR = (uint16_t)interrupt_mask;
  return USBD_OK;
}

USBD_StatusTypeDef USB_DevConnect(USB_TypeDef *USBx)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(USBx);
  return USBD_OK;
}

USBD_StatusTypeDef USB_DevDisconnect(USB_TypeDef *USBx)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(USBx);
  return USBD_OK;
}

/** Встановлює адресу USB-пристрою (0-127) */
USBD_StatusTypeDef USB_SetDevAddress(USB_TypeDef *USBx, uint8_t address)
{
  if (address == 0U)
  {
    /* set device address and enable function */
    USBx->DADDR = (uint16_t)USB_DADDR_EF;
  }
  return USBD_OK;
}

/** Читання активних переривань */
uint32_t USB_ReadInterrupts(const USB_TypeDef *USBx)
{
  return USBx->ISTR;
}

void USB_ReadPMA(USB_TypeDef const *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = (uint32_t)wNBytes >> 1;
  uint32_t BaseAddr = (uint32_t)USBx;
  uint32_t count;
  uint32_t RdVal;
  __IO uint16_t *pdwVal;
  uint8_t *pBuf = pbUsrBuf;

  pdwVal = (__IO uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * PMA_ACCESS));

  for (count = n; count != 0U; count--)
  {
    RdVal = *(__IO uint16_t *)pdwVal;
    pdwVal++;
    *pBuf = (uint8_t)((RdVal >> 0) & 0xFFU);
    pBuf++;
    *pBuf = (uint8_t)((RdVal >> 8) & 0xFFU);
    pBuf++;
    #if PMA_ACCESS > 1U
    pdwVal++;
    #endif /* PMA_ACCESS */
  }
  if ((wNBytes % 2U) != 0U)
  {
    RdVal = *pdwVal;
    *pBuf = (uint8_t)((RdVal >> 0) & 0xFFU);
  }
}

void USB_WritePMA(USB_TypeDef const *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = ((uint32_t)wNBytes + 1U) >> 1;
  uint32_t BaseAddr = (uint32_t)USBx;
  uint32_t count;
  uint16_t WrVal;
  __IO uint16_t *pdwVal;
  uint8_t *pBuf = pbUsrBuf;

  pdwVal = (__IO uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * PMA_ACCESS));

  for (count = n; count != 0U; count--)
  {
    WrVal = pBuf[0];
    WrVal |= (uint16_t)pBuf[1] << 8;
    *pdwVal = (WrVal & 0xFFFFU);
    pdwVal++;
    #if PMA_ACCESS > 1U
    pdwVal++;
    #endif /* PMA_ACCESS */
    pBuf++;
    pBuf++;
  }
}

USBD_StatusTypeDef USB_ActivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  USBD_StatusTypeDef ret = USBD_OK;
  uint16_t wEpRegVal;
  wEpRegVal = USBD_PCD_GET_ENDPOINT(USBx, ep->num) & USB_EP_T_MASK;
  /* initialize Endpoint */
  switch (ep->type)
  {
    case EP_TYPE_CTRL:
    wEpRegVal |= USB_EP_CONTROL;
    break;
    case EP_TYPE_BULK:
    wEpRegVal |= USB_EP_BULK;
    break;
    case EP_TYPE_INTR:
    wEpRegVal |= USB_EP_INTERRUPT;
    break;
    case EP_TYPE_ISOC:
    wEpRegVal |= USB_EP_ISOCHRONOUS;
    break;
    default:
    ret = USBD_FAIL;
    break;
  }
  USBD_PCD_SET_ENDPOINT(USBx, ep->num, (wEpRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  USBD_PCD_SET_EP_ADDRESS(USBx, ep->num, ep->num);
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      /*Set the endpoint Transmit buffer address */
      USBD_PCD_SET_EP_TX_ADDRESS(USBx, ep->num, ep->pmaadress);
      USBD_PCD_CLEAR_TX_DTOG(USBx, ep->num);
      if (ep->type != EP_TYPE_ISOC)
      {
        /* Configure NAK status for the Endpoint */
        USBD_PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
      else
      {
        /* Configure TX Endpoint to disabled state */
        USBD_PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
      }
    }
    else
    {
      /* Set the endpoint Receive buffer address */
      USBD_PCD_SET_EP_RX_ADDRESS(USBx, ep->num, ep->pmaadress);
      /* Set the endpoint Receive buffer counter */
      USBD_PCD_SET_EP_RX_CNT(USBx, ep->num, ep->maxpacket);
      USBD_PCD_CLEAR_RX_DTOG(USBx, ep->num);
      if (ep->num == 0U)
      {
        /* Configure VALID status for EP0 */
        USBD_PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
      }
      else
      {
        /* Configure NAK status for OUT Endpoint */
        USBD_PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_NAK);
      }
    }
  }
  return ret;
}

USBD_StatusTypeDef USB_DeactivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  USBD_PCD_CLEAR_TX_DTOG(USBx, ep->num);
  USBD_PCD_CLEAR_RX_DTOG(USBx, ep->num);
  /* Configure DISABLE status for the Endpoint */
  USBD_PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
  USBD_PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
  return USBD_OK;
}

USBD_StatusTypeDef USB_EPStartXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  uint32_t len;
  /* IN endpoint */
  if (ep->is_in == 1U)
  {
    /*Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket)
    {
      len = ep->maxpacket;
    }
    else
    {
      len = ep->xfer_len;
    }
    /* configure and validate Tx endpoint */
    if (ep->doublebuffer == 0U)
    {
      USB_WritePMA(USBx, ep->xfer_buff, ep->pmaadress, (uint16_t)len);
      USBD_PCD_SET_EP_TX_CNT(USBx, ep->num, len);
    }
    USBD_PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_VALID);
  }
  else /* OUT endpoint */
  {
    if (ep->doublebuffer == 0U)
    {
      /* Multi packet transfer */
      if (ep->xfer_len > ep->maxpacket)
      {
        len = ep->maxpacket;
        ep->xfer_len -= len;
      }
      else
      {
        len = ep->xfer_len;
        ep->xfer_len = 0U;
      }
      /* configure and validate Rx endpoint */
      USBD_PCD_SET_EP_RX_CNT(USBx, ep->num, len);
    }
    USBD_PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
  }
  return USBD_OK;
}

USBD_StatusTypeDef USB_EPSetStall(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->is_in != 0U)
  {
    USBD_PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_STALL);
  }
  else
  {
    USBD_PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_STALL);
  }
  return USBD_OK;
}

USBD_StatusTypeDef USB_EP0_OutStart(USB_TypeDef *USBx, uint8_t *psetup)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(USBx);
  (void)(psetup);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
  only by USB OTG FS peripheral.
  - This function is added to ensure compatibility across platforms. */
  return USBD_OK;
}

USBD_StatusTypeDef USB_EPClearStall(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      USBD_PCD_CLEAR_TX_DTOG(USBx, ep->num);
      if (ep->type != EP_TYPE_ISOC)
      {
        /* Configure NAK status for the Endpoint */
        USBD_PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
    }
    else
    {
      USBD_PCD_CLEAR_RX_DTOG(USBx, ep->num);
      /* Configure VALID status for the Endpoint */
      USBD_PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
    }
  }
  return USBD_OK;
}

