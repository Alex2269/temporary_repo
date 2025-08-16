// stm32f1xx_hal_pcd.c

// #include "usbd_core.h"
#include "usbd_cdc.h"

// Попереднє оголошення допоміжних функцій для обробки ендпоінт.
static USBD_StatusTypeDef Handle_EP0_IN(PCD_HandleTypeDef *hpcd);
static USBD_StatusTypeDef Handle_EP0_OUT_SETUP(PCD_HandleTypeDef *hpcd, uint16_t wIstr);
static USBD_StatusTypeDef Handle_OUT_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal);
static USBD_StatusTypeDef Handle_IN_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal);

// === callback обгортки видалені ===

// Заміна викликів callback-функцій напряму на USBD_LL_ функції у коді...

USBD_StatusTypeDef USBD_PCDEx_PMAConfig(PCD_HandleTypeDef *hpcd, uint16_t ep_addr, uint16_t ep_kind, uint32_t pmaadress)
{
  USBD_PCD_EPTypeDef *ep;

  if ((0x80U & ep_addr) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr];
  }

  if (ep_kind == USBD_PCD_SNG_BUF)
  {
    ep->doublebuffer = 0U;
    ep->pmaadress = (uint16_t)pmaadress;
  }
  return USBD_OK;
}

void USBD_PCD_IRQHandler(PCD_HandleTypeDef *hpcd)
{
  uint32_t wIstr = USB_ReadInterrupts(hpcd->Instance);

  if ((wIstr & USB_ISTR_CTR) == USB_ISTR_CTR)
  {
    uint16_t istr = hpcd->Instance->ISTR;
    uint8_t epindex = (uint8_t)(istr & USB_ISTR_EP_ID);

    if ((istr & USB_ISTR_CTR) == 0U)
      return;

    if (epindex == 0U)
    {
      if ((istr & USB_ISTR_DIR) == 0U)
      {
        (void)Handle_EP0_IN(hpcd);
      }
      else
      {
        (void)Handle_EP0_OUT_SETUP(hpcd, istr);
      }
    }
    else
    {
      uint16_t wEPVal = USBD_PCD_GET_ENDPOINT(hpcd->Instance, epindex);
      if ((wEPVal & USB_EP_CTR_RX) != 0U)
      {
        (void)Handle_OUT_Transfer(hpcd, epindex, wEPVal);
      }
      if ((wEPVal & USB_EP_CTR_TX) != 0U)
      {
        (void)Handle_IN_Transfer(hpcd, epindex, wEPVal);
      }
    }
    return;
  }

  if ((wIstr & USB_ISTR_RESET) == USB_ISTR_RESET)
  {
    CLEAR_BIT(hpcd->Instance->ISTR, USB_ISTR_RESET);
    USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
    if (hpcd->Init.speed != USBD_PCD_SPEED_FULL)
    {
      Error_Handler();
    }
    USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);
    USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);

    (void)USBD_PCD_SetAddress(hpcd, 0U);
  }

  CLEAR_BIT(hpcd->Instance->ISTR, USB_ISTR_PMAOVR | USB_ISTR_ERR | USB_ISTR_WKUP | USB_ISTR_RESET | USB_ISTR_SUSP | USB_ISTR_SOF | USB_ISTR_ESOF);
}

USBD_StatusTypeDef USBD_PCD_SetAddress(PCD_HandleTypeDef *hpcd, uint8_t address)
{
  hpcd->USB_Address = address;
  (void)USB_SetDevAddress(hpcd->Instance, address);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_Init(PCD_HandleTypeDef *hpcd)
{
  USBD_PCD_MspInit(hpcd);
  (void)USB_DisableGlobalInt(hpcd->Instance);

  for (uint8_t i = 0U; i < hpcd->Init.dev_endpoints; i++)
  {
    hpcd->IN_ep[i] = (USBD_PCD_EPTypeDef){.is_in = 1U, .num = i, .type = EP_TYPE_CTRL};
    hpcd->OUT_ep[i] = (USBD_PCD_EPTypeDef){.is_in = 0U, .num = i, .type = EP_TYPE_CTRL};
  }
  hpcd->USB_Address = 0U;
  hpcd->State = USBD_PCD_STATE_READY;
  (void)USB_DevDisconnect(hpcd->Instance);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_Start(PCD_HandleTypeDef *hpcd)
{
  (void)USB_EnableGlobalInt(hpcd->Instance);
  (void)USB_DevConnect(hpcd->Instance);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_EP_Open(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint16_t ep_mps, uint8_t ep_type)
{
  USBD_PCD_EPTypeDef *ep = (ep_addr & 0x80U) ? &hpcd->IN_ep[ep_addr & EP_ADDR_MSK] : &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
  ep->is_in = (ep_addr & 0x80U) != 0U;
  ep->num = ep_addr & EP_ADDR_MSK;
  ep->maxpacket = ep_mps;
  ep->type = ep_type;
  (void)USB_ActivateEndpoint(hpcd->Instance, ep);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_EP_Close(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  USBD_PCD_EPTypeDef *ep = (ep_addr & 0x80U) ? &hpcd->IN_ep[ep_addr & EP_ADDR_MSK] : &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
  ep->is_in = (ep_addr & 0x80U) != 0U;
  ep->num = ep_addr & EP_ADDR_MSK;
  (void)USB_DeactivateEndpoint(hpcd->Instance, ep);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_EP_SetStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  USBD_PCD_EPTypeDef *ep = (ep_addr & 0x80U) ? &hpcd->IN_ep[ep_addr & EP_ADDR_MSK] : &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
  ep->is_in = (ep_addr & 0x80U) != 0U;
  ep->is_stall = 1U;
  ep->num = ep_addr & EP_ADDR_MSK;
  (void)USB_EPSetStall(hpcd->Instance, ep);
  if (ep_addr == 0U)
    (void)USB_EP0_OutStart(hpcd->Instance, (uint8_t *)hpcd->Setup);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_EP_Transmit(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
  ep->xfer_buff = pBuf;
  ep->xfer_len = len;
  ep->xfer_count = 0U;
  ep->is_in = 1U;
  ep->num = ep_addr & EP_ADDR_MSK;
  (void)USB_EPStartXfer(hpcd->Instance, ep);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_EP_Receive(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
  ep->xfer_buff = pBuf;
  ep->xfer_len = len;
  ep->xfer_count = 0U;
  ep->is_in = 0U;
  ep->num = ep_addr & EP_ADDR_MSK;
  (void)USB_EPStartXfer(hpcd->Instance, ep);
  return USBD_OK;
}

uint32_t USBD_PCD_EP_GetRxCount(const PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  return hpcd->OUT_ep[ep_addr & EP_ADDR_MSK].xfer_count;
}

// Обробник для IN-передачі для EP0
static USBD_StatusTypeDef Handle_EP0_IN(PCD_HandleTypeDef *hpcd)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->IN_ep[0];
  USBD_PCD_CLEAR_TX_EP_CTR(hpcd->Instance, USBD_PCD_ENDP0);
  ep->xfer_count = USBD_PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num);
  ep->xfer_buff += ep->xfer_count;
  // Прямий виклик LL функції без callback обгортки
  USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, 0U, ep->xfer_buff);
  if ((hpcd->USB_Address > 0U) && (ep->xfer_len == 0U))
  {
    hpcd->Instance->DADDR = ((uint16_t)hpcd->USB_Address | USB_DADDR_EF);
    hpcd->USB_Address = 0U;
  }
  return USBD_OK;
}

// Обробник для OUT/SETUP передачі для EP0
static USBD_StatusTypeDef Handle_EP0_OUT_SETUP(PCD_HandleTypeDef *hpcd, uint16_t wIstr)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[0];
  uint16_t wEPVal = USBD_PCD_GET_ENDPOINT(hpcd->Instance, USBD_PCD_ENDP0);
  if ((wEPVal & USB_EP_SETUP) != 0U)
  {
    ep->xfer_count = USBD_PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
    USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, (uint16_t)ep->xfer_count);
    USBD_PCD_CLEAR_RX_EP_CTR(hpcd->Instance, USBD_PCD_ENDP0);
    // Прямий виклик LL функції без callback обгортки
    USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
  }
  else if ((wEPVal & USB_EP_CTR_RX) != 0U)
  {
    USBD_PCD_CLEAR_RX_EP_CTR(hpcd->Instance, USBD_PCD_ENDP0);
    ep->xfer_count = USBD_PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
    if ((ep->xfer_count != 0U) && (ep->xfer_buff != 0U))
    {
      USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, (uint16_t)ep->xfer_count);
      ep->xfer_buff += ep->xfer_count;
      // Прямий виклик LL функції без callback обгортки
      USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, 0U, ep->xfer_buff);
    }
    wEPVal = (uint16_t)USBD_PCD_GET_ENDPOINT(hpcd->Instance, USBD_PCD_ENDP0);
    if (((wEPVal & USB_EP_SETUP) == 0U) && ((wEPVal & USB_EP_RX_STRX) != USB_EP_RX_VALID))
    {
      USBD_PCD_SET_EP_RX_CNT(hpcd->Instance, USBD_PCD_ENDP0, ep->maxpacket);
    }
  }
  return USBD_OK;
}

// Обробка OUT передачі для неконтрольного ендпоінта
static USBD_StatusTypeDef Handle_OUT_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->OUT_ep[epindex];
  uint16_t count = USBD_PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
  USBD_PCD_CLEAR_RX_EP_CTR(hpcd->Instance, epindex);
  if (count != 0U)
  {
    USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, count);
  }
  ep->xfer_count += count;
  ep->xfer_buff += count;
  if ((ep->xfer_len == 0U) || (count < ep->maxpacket))
  {
    USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, ep->num, ep->xfer_buff);
  }
  else
  {
    (void)USB_EPStartXfer(hpcd->Instance, ep);
  }
  return USBD_OK;
}

// Обробка IN передачі для неконтрольного ендпоінта
static USBD_StatusTypeDef Handle_IN_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal)
{
  USBD_PCD_EPTypeDef *ep = &hpcd->IN_ep[epindex];
  USBD_PCD_CLEAR_TX_EP_CTR(hpcd->Instance, epindex);
  if (ep->type == EP_TYPE_ISOC)
  {
    ep->xfer_len = 0U;
    USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, ep->num, ep->xfer_buff);
  }
  else
  {
    if ((wEPVal & USB_EP_KIND) == 0U)
    {
      uint16_t TxPctSize = USBD_PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num);
      if (ep->xfer_len > TxPctSize)
      {
        ep->xfer_len -= TxPctSize;
      }
      else
      {
        ep->xfer_len = 0U;
      }
      if (ep->xfer_len == 0U)
      {
        USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, ep->num, ep->xfer_buff);
      }
      else
      {
        ep->xfer_buff += TxPctSize;
        ep->xfer_count += TxPctSize;
        (void)USB_EPStartXfer(hpcd->Instance, ep);
      }
    }
  }
  return USBD_OK;
}

USBD_StatusTypeDef USBD_PCD_EP_ClrStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  return USBD_OK; // Поки що не реалізовано
}

