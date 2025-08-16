// usbd_conf.c

#include "stm32f1xx.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

PCD_HandleTypeDef hpcd_USB_FS;

void Error_Handler(void);

/* MSP Init */
void USBD_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{
  if(pcdHandle->Instance==USB)
  {
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USBEN);
    while (READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USBEN) == 0) {}

    NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0);
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  }
}

void USBD_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{
  if(pcdHandle->Instance==USB)
  {
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
    NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  }
}

/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef * pdev)
{
  /* Init USB Ip. */
  /* Link the driver to the stack. */
  hpcd_USB_FS.pData = pdev;
  pdev->pData = &hpcd_USB_FS;

  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = USBD_PCD_SPEED_FULL;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (USBD_PCD_Init(&hpcd_USB_FS) != USBD_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN EndPoint_Configuration */
  USBD_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x00 , USBD_PCD_SNG_BUF, 0x18);
  USBD_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x80 , USBD_PCD_SNG_BUF, 0x58);
  /* USER CODE END EndPoint_Configuration */
  /* USER CODE BEGIN EndPoint_Configuration_CDC */
  USBD_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x81 , USBD_PCD_SNG_BUF, 0xC0);
  USBD_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x01 , USBD_PCD_SNG_BUF, 0x110);
  USBD_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x82 , USBD_PCD_SNG_BUF, 0x100);
  /* USER CODE END EndPoint_Configuration_CDC */
  return USBD_OK;
}

/**
  * @brief  De-Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef * pdev)
{
  return USBD_OK;
}

/**
  * @brief  Starts the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef * pdev)
{
  USBD_PCD_Start((PCD_HandleTypeDef *) pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Stops the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef * pdev)
{
  USBD_PCD_Stop((PCD_HandleTypeDef *) pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Opens an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  ep_type: Endpoint Type
  * @param  ep_mps: Endpoint Max Packet Size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr,
                                  uint8_t ep_type, uint16_t ep_mps)
{
  USBD_PCD_EP_Open((PCD_HandleTypeDef *) pdev->pData, ep_addr, ep_mps, ep_type);

  return USBD_OK;
}

/**
  * @brief  Closes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  USBD_PCD_EP_Close((PCD_HandleTypeDef *) pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  USBD_PCD_EP_Flush((PCD_HandleTypeDef *) pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  USBD_PCD_EP_SetStall((PCD_HandleTypeDef *) pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  USBD_PCD_EP_ClrStall((PCD_HandleTypeDef *) pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef *) pdev->pData;

  if ((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef * pdev, uint8_t dev_addr)
{
  USBD_PCD_SetAddress((PCD_HandleTypeDef *) pdev->pData, dev_addr);
  return USBD_OK;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef * pdev, uint8_t ep_addr,
                                    uint8_t * pbuf, uint16_t size)
{
  USBD_PCD_EP_Transmit((PCD_HandleTypeDef *) pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef * pdev, uint8_t ep_addr,
                                          uint8_t * pbuf, uint16_t size)
{
  USBD_PCD_EP_Receive((PCD_HandleTypeDef *) pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Returns the last transferred packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Received Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  return USBD_PCD_EP_GetRxCount((PCD_HandleTypeDef *) pdev->pData, ep_addr);
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  LL_mDelay(Delay);
}

void *USBD_static_malloc(uint32_t size)
{
  static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef)/4)+1];/* On 32-bit boundary */
  return mem;
}

void USBD_static_free(void *p)
{

}

