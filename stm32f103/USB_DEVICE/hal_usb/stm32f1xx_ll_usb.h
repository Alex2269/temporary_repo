// file  stm32f1xx_ll_usb.h

#ifndef STM32F1xx_LL_USB_H
#define STM32F1xx_LL_USB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stm32f1xx.h"
#include <stddef.h>


#if defined (USB) || defined (USB_OTG_FS)


#ifndef HAL_USB_TIMEOUT
#define HAL_USB_TIMEOUT                                       0xF000000U
#endif /* define HAL_USB_TIMEOUT */


#ifndef HAL_USB_CURRENT_MODE_MAX_DELAY_MS
#define HAL_USB_CURRENT_MODE_MAX_DELAY_MS                           200U
#endif /* define HAL_USB_CURRENT_MODE_MAX_DELAY_MS */


/**
  * @brief  USB Mode definition
  */

typedef enum
{
  USBD_OK   = 0U,
  USBD_BUSY,
  USBD_FAIL,
} USBD_StatusTypeDef;

typedef enum
{
  USB_DEVICE_MODE = 0,
  USB_HOST_MODE   = 1,
  USB_DRD_MODE    = 2
} USB_ModeTypeDef;

/**
  * @brief  URB States definition
  */
typedef enum
{
  URB_IDLE = 0,
  URB_DONE,
  URB_NOTREADY,
  URB_NYET,
  URB_ERROR,
  URB_STALL
} USB_URBStateTypeDef;

/**
  * @brief  Host channel States  definition
  */
typedef enum
{
  HC_IDLE = 0,
  HC_XFRC,
  HC_HALTED,
  HC_ACK,
  HC_NAK,
  HC_NYET,
  HC_STALL,
  HC_XACTERR,
  HC_BBLERR,
  HC_DATATGLERR
} USB_HCStateTypeDef;

/**
  * @brief  USB Instance Initialization Structure definition
  */
typedef struct
{
  uint8_t dev_endpoints;            /*!< Device Endpoints number.
                                         This parameter depends on the used USB core.
                                         This parameter must be a number between Min_Data = 1 and Max_Data = 15 */
  uint8_t dma_enable;              /*!< USB DMA state.
                                         If DMA is not supported this parameter shall be set by default to zero */

  uint8_t speed;                   /*!< USB Core speed.
                                        This parameter can be any value of @ref USBD_PCD_Speed/HCD_Speed
                                                                                (HCD_SPEED_xxx, HCD_SPEED_xxx) */

  uint8_t ep0_mps;                 /*!< Set the Endpoint 0 Max Packet size.                                    */

  uint8_t phy_itface;              /*!< Select the used PHY interface.
                                        This parameter can be any value of @ref USBD_PCD_PHY_Module/HCD_PHY_Module  */

  uint8_t Sof_enable;              /*!< Enable or disable the output of the SOF signal.                        */

  uint8_t low_power_enable;        /*!< Enable or disable the low Power Mode.                                  */

  uint8_t lpm_enable;              /*!< Enable or disable Link Power Management.                               */

  uint8_t battery_charging_enable; /*!< Enable or disable Battery charging.                                    */
} USB_CfgTypeDef;

typedef struct
{
  uint8_t   num;                  /*!< Endpoint number
                                       This parameter must be a number between Min_Data = 1 and Max_Data = 15   */

  uint8_t   is_in;                /*!< Endpoint direction
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 1    */

  uint8_t   is_stall;             /*!< Endpoint stall condition
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 1    */
  uint8_t   type;                 /*!< Endpoint type
                                       This parameter can be any value of @ref USB_LL_EP_Type                   */

  uint8_t   data_pid_start;       /*!< Initial data PID
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 1    */

#if defined (USB)
  uint16_t  pmaadress;            /*!< PMA Address
                                       This parameter can be any value between Min_addr = 0 and Max_addr = 1K   */

  uint16_t  pmaaddr0;             /*!< PMA Address0
                                       This parameter can be any value between Min_addr = 0 and Max_addr = 1K   */

  uint16_t  pmaaddr1;             /*!< PMA Address1
                                       This parameter can be any value between Min_addr = 0 and Max_addr = 1K   */

  uint8_t   doublebuffer;         /*!< Double buffer enable
                                       This parameter can be 0 or 1                                             */
#endif /* defined (USB) */

  uint32_t  maxpacket;            /*!< Endpoint Max packet size
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 64KB */

  uint8_t   *xfer_buff;           /*!< Pointer to transfer buffer                                               */

  uint32_t  xfer_len;             /*!< Current transfer length                                                  */

  uint32_t  xfer_count;           /*!< Partial transfer length in case of multi packet transfer                 */

#if defined (USB)
  uint32_t  xfer_len_db;          /*!< double buffer transfer length used with bulk double buffer in            */

  uint8_t   xfer_fill_db;         /*!< double buffer Need to Fill new buffer  used with bulk_in                 */
#endif /* defined (USB) */
} USB_EPTypeDef;

/** @defgroup USB_LL_EP0_MPS USB Low Layer EP0 MPS
  * @{
  */
#define EP_MPS_64                              0U
#define EP_MPS_32                              1U
#define EP_MPS_16                              2U
#define EP_MPS_8                               3U
/**
  * @}
  */

/** @defgroup USB_LL_EP_Type USB Low Layer EP Type
  * @{
  */
#define EP_TYPE_CTRL                           0U
#define EP_TYPE_ISOC                           1U
#define EP_TYPE_BULK                           2U
#define EP_TYPE_INTR                           3U
#define EP_TYPE_MSK                            3U
/**
  * @}
  */

/** @defgroup USB_LL_EP_Speed USB Low Layer EP Speed
  * @{
  */
#define EP_SPEED_LOW                           0U
#define EP_SPEED_FULL                          1U
#define EP_SPEED_HIGH                          2U
/**
  * @}
  */

/** @defgroup USB_LL_CH_PID_Type USB Low Layer Channel PID Type
  * @{
  */
#define HC_PID_DATA0                           0U
#define HC_PID_DATA2                           1U
#define HC_PID_DATA1                           2U
#define HC_PID_SETUP                           3U
/**
  * @}
  */

/** @defgroup USB_LL Device Speed
  * @{
  */
#define USBD_FS_SPEED                          2U
#define USBH_FSLS_SPEED                        1U


#if defined (USB)
#define BTABLE_ADDRESS                         0x000U
#define PMA_ACCESS                             2U

#ifndef USB_EP_RX_STRX
#define USB_EP_RX_STRX                         (0x3U << 12)
#endif /* USB_EP_RX_STRX */

#define EP_ADDR_MSK                            0x7U

#endif /* defined (USB) */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup USB_LL_Exported_Functions USB Low Layer Exported Functions
  * @{
  */

#if defined (USB)
USBD_StatusTypeDef USB_CoreInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg);
USBD_StatusTypeDef USB_DevInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg);
USBD_StatusTypeDef USB_EnableGlobalInt(USB_TypeDef *USBx);
USBD_StatusTypeDef USB_DisableGlobalInt(USB_TypeDef *USBx);
USBD_StatusTypeDef USB_SetCurrentMode(USB_TypeDef *USBx, USB_ModeTypeDef mode);
USBD_StatusTypeDef USB_SetDevSpeed(USB_TypeDef *USBx, uint8_t speed);

USBD_StatusTypeDef USB_FlushRxFifo(USB_TypeDef const *USBx);
USBD_StatusTypeDef USB_FlushTxFifo(USB_TypeDef const *USBx, uint32_t num);


USBD_StatusTypeDef USB_ActivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep);
USBD_StatusTypeDef USB_DeactivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep);
USBD_StatusTypeDef USB_EPStartXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep);
USBD_StatusTypeDef USB_EPSetStall(USB_TypeDef *USBx, USB_EPTypeDef *ep);
USBD_StatusTypeDef USB_EPClearStall(USB_TypeDef *USBx, USB_EPTypeDef *ep);
USBD_StatusTypeDef USB_EPStopXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep);


USBD_StatusTypeDef USB_SetDevAddress(USB_TypeDef *USBx, uint8_t address);
USBD_StatusTypeDef USB_DevConnect(USB_TypeDef *USBx);
USBD_StatusTypeDef USB_DevDisconnect(USB_TypeDef *USBx);
USBD_StatusTypeDef USB_StopDevice(USB_TypeDef *USBx);
USBD_StatusTypeDef USB_EP0_OutStart(USB_TypeDef *USBx, uint8_t *psetup);
USBD_StatusTypeDef USB_WritePacket(USB_TypeDef *USBx, uint8_t *src,
                                  uint8_t ch_ep_num, uint16_t len);

void             *USB_ReadPacket(USB_TypeDef *USBx, uint8_t *dest, uint16_t len);

uint32_t          USB_ReadInterrupts(USB_TypeDef const *USBx);
uint32_t          USB_ReadDevAllOutEpInterrupt(USB_TypeDef *USBx);
uint32_t          USB_ReadDevOutEPInterrupt(USB_TypeDef *USBx, uint8_t epnum);
uint32_t          USB_ReadDevAllInEpInterrupt(USB_TypeDef *USBx);
uint32_t          USB_ReadDevInEPInterrupt(USB_TypeDef *USBx, uint8_t epnum);
void              USB_ClearInterrupts(USB_TypeDef *USBx, uint32_t interrupt);
USBD_StatusTypeDef USB_ActivateRemoteWakeup(USB_TypeDef *USBx);
USBD_StatusTypeDef USB_DeActivateRemoteWakeup(USB_TypeDef *USBx);

void              USB_WritePMA(USB_TypeDef const *USBx, uint8_t *pbUsrBuf,
                               uint16_t wPMABufAddr, uint16_t wNBytes);

void              USB_ReadPMA(USB_TypeDef const *USBx, uint8_t *pbUsrBuf,
                              uint16_t wPMABufAddr, uint16_t wNBytes);
#endif /* defined (USB) */


#endif /* defined (USB) || defined (USB_OTG_FS) */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* STM32F1xx_LL_USB_H */
