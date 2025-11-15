// stm32f1xx_hal_pcd.h

#ifndef STM32F1xx_USBD_PCD_H
#define STM32F1xx_USBD_PCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_ll_usb.h"

#if defined (USB)

typedef enum
{
  USBD_PCD_STATE_RESET   = 0x00,
  USBD_PCD_STATE_READY   = 0x01,
  USBD_PCD_STATE_ERROR   = 0x02,
  USBD_PCD_STATE_BUSY    = 0x03,
  USBD_PCD_STATE_TIMEOUT = 0x04
} USBD_PCD_StateTypeDef;

/* Device LPM suspend state */
typedef enum
{
  LPM_L0 = 0x00, /* on */
  LPM_L1 = 0x01, /* LPM L1 sleep */
  LPM_L2 = 0x02, /* suspend */
  LPM_L3 = 0x03, /* off */
} USBD_PCD_LPM_StateTypeDef;

typedef enum
{
  USBD_PCD_LPM_L0_ACTIVE = 0x00, /* on */
  USBD_PCD_LPM_L1_ACTIVE = 0x01, /* LPM L1 sleep */
} USBD_PCD_LPM_MsgTypeDef;

typedef USB_TypeDef        USBD_PCD_TypeDef;
typedef USB_CfgTypeDef     USBD_PCD_InitTypeDef;
typedef USB_EPTypeDef      USBD_PCD_EPTypeDef;


/**
  * @brief  PCD Handle Structure definition
  */
typedef struct
{
  USBD_PCD_TypeDef             *Instance;   /*!< Register base address             */
  USBD_PCD_InitTypeDef         Init;        /*!< PCD required parameters           */
  __IO uint8_t                 USB_Address; /*!< USB Address                       */

  USBD_PCD_EPTypeDef           IN_ep[8];    /*!< IN endpoint parameters            */
  USBD_PCD_EPTypeDef           OUT_ep[8];   /*!< OUT endpoint parameters           */

  uint8_t                      Lock;        /*!< PCD peripheral status             */
  __IO USBD_PCD_StateTypeDef   State;       /*!< PCD communication state           */
  __IO  uint32_t               ErrorCode;   /*!< PCD Error code                    */
  uint32_t                     Setup[12];   /*!< Setup packet buffer               */
  USBD_PCD_LPM_StateTypeDef    LPM_State;   /*!< LPM State                         */
  uint32_t                     BESL;
  uint32_t                     FrameNumber; /*!< Store Current Frame number        */
  void                        *pData;      /*!< Pointer to upper stack Handler */
} PCD_HandleTypeDef;


void USBD_PCD_MspInit(PCD_HandleTypeDef *hpcd);
void USBD_PCD_MspDeInit(PCD_HandleTypeDef *hpcd);

static inline void USBD_PCD_RX_DTOG(USB_TypeDef* USBx, uint8_t bEpNum);
static inline void USBD_PCD_TX_DTOG(USB_TypeDef* USBx, uint8_t bEpNum);

// Основні функції керування PCD
USBD_StatusTypeDef USBD_PCDEx_PMAConfig(PCD_HandleTypeDef *hpcd, uint16_t ep_addr, uint16_t ep_kind, uint32_t pmaadress);
void USBD_PCD_IRQHandler(PCD_HandleTypeDef *hpcd);
USBD_StatusTypeDef USBD_PCD_SetAddress(PCD_HandleTypeDef *hpcd, uint8_t address);
USBD_StatusTypeDef USBD_PCD_Init(PCD_HandleTypeDef *hpcd);
USBD_StatusTypeDef USBD_PCD_Start(PCD_HandleTypeDef *hpcd);
USBD_StatusTypeDef USBD_PCD_Stop(PCD_HandleTypeDef *hpcd);
USBD_StatusTypeDef USBD_PCD_EP_Flush(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
USBD_StatusTypeDef USBD_PCD_EP_Open(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint16_t ep_mps, uint8_t ep_type);
USBD_StatusTypeDef USBD_PCD_EP_Close(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
USBD_StatusTypeDef USBD_PCD_EP_SetStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
USBD_StatusTypeDef USBD_PCD_EP_Transmit(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len);
USBD_StatusTypeDef USBD_PCD_EP_Receive(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len);
uint32_t USBD_PCD_EP_GetRxCount(const PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
USBD_StatusTypeDef USBD_PCD_EP_ClrStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);

// Прототипи внутрішніх статичних функцій (можна не додавати у заголовок,
// якщо вони не використовуються поза файлом stm32f1xx_hal_pcd.c,
// але для повноти тут наведені)
static USBD_StatusTypeDef Handle_EP0_IN(PCD_HandleTypeDef *hpcd);
static USBD_StatusTypeDef Handle_EP0_OUT_SETUP(PCD_HandleTypeDef *hpcd, uint16_t wIstr);
static USBD_StatusTypeDef Handle_OUT_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal);
static USBD_StatusTypeDef Handle_IN_Transfer(PCD_HandleTypeDef *hpcd, uint8_t epindex, uint16_t wEPVal);


#define USBD_PCD_SPEED_FULL               USBD_FS_SPEED

#define USBD_PCD_PHY_ULPI                 1U
#define USBD_PCD_PHY_EMBEDDED             2U
#define USBD_PCD_PHY_UTMI                 3U

#define __USBD_PCD_ENABLE(__HANDLE__)                       (void)USB_EnableGlobalInt ((__HANDLE__)->Instance)
#define __USBD_PCD_DISABLE(__HANDLE__)                      (void)USB_DisableGlobalInt ((__HANDLE__)->Instance)

#define __USBD_PCD_GET_FLAG(__HANDLE__, __INTERRUPT__) \
  ((USB_ReadInterrupts((__HANDLE__)->Instance) & (__INTERRUPT__)) == (__INTERRUPT__))

#define __USBD_PCD_CLEAR_FLAG(__HANDLE__, __INTERRUPT__)           (((__HANDLE__)->Instance->ISTR)\
                                                                   &= (uint16_t)(~(__INTERRUPT__)))

#define USB_WAKEUP_EXTI_LINE                                     (0x1U << 18)  /*!< USB FS EXTI Line WakeUp Interrupt */

#define __HAL_USB_WAKEUP_EXTI_ENABLE_IT()                         EXTI->IMR |= USB_WAKEUP_EXTI_LINE
#define __HAL_USB_WAKEUP_EXTI_DISABLE_IT()                        EXTI->IMR &= ~(USB_WAKEUP_EXTI_LINE)
#define __HAL_USB_WAKEUP_EXTI_GET_FLAG()                          EXTI->PR & (USB_WAKEUP_EXTI_LINE)
#define __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG()                        EXTI->PR = USB_WAKEUP_EXTI_LINE


static inline void
__HAL_USB_WAKEUP_EXTI_ENABLE_RISING_EDGE(void)
{
  do {
    EXTI->FTSR &= ~(USB_WAKEUP_EXTI_LINE);
    EXTI->RTSR |= (USB_WAKEUP_EXTI_LINE);
  } while(0U);
}

#define USBD_PCD_EP0MPS_64                                                 EP_MPS_64
#define USBD_PCD_EP0MPS_32                                                 EP_MPS_32
#define USBD_PCD_EP0MPS_16                                                 EP_MPS_16
#define USBD_PCD_EP0MPS_08                                                 EP_MPS_8

#define USBD_PCD_ENDP0                                                     0U
#define USBD_PCD_ENDP1                                                     1U
#define USBD_PCD_ENDP2                                                     2U
#define USBD_PCD_ENDP3                                                     3U
#define USBD_PCD_ENDP4                                                     4U
#define USBD_PCD_ENDP5                                                     5U
#define USBD_PCD_ENDP6                                                     6U
#define USBD_PCD_ENDP7                                                     7U

#define USBD_PCD_SNG_BUF                                                   0U
#define USBD_PCD_DBL_BUF                                                   1U

/********************  Bit definition for USB_COUNTn_RX register  *************/
#define USB_CNTRX_NBLK_MSK                    (0x1FU << 10)
#define USB_CNTRX_BLSIZE                      (0x1U << 15)

/* SetENDPOINT */
static inline uint16_t
USBD_PCD_SET_ENDPOINT(USB_TypeDef* USBx, uint8_t bEpNum, uint32_t wRegValue) {
  return   (*(volatile uint16_t *)(&(USBx)->EP0R + ((bEpNum) * 2U)) = (uint16_t)(wRegValue));
}

/* GetENDPOINT */
static inline uint16_t
USBD_PCD_GET_ENDPOINT(USB_TypeDef* USBx, uint8_t bEpNum) {
  return (*(volatile uint16_t*)(&(USBx)->EP0R + ((uint32_t)bEpNum * 2U)));
}

/**
  * @brief  sets the type in the endpoint register(bits EP_TYPE[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wType Endpoint Type.
  * @retval None
  */

static inline uint16_t
USBD_PCD_SET_EPTYPE(USB_TypeDef* USBx, uint8_t bEpNum, uint32_t wType) {
  return (USBD_PCD_SET_ENDPOINT((USBx), (bEpNum),
                               ((USBD_PCD_GET_ENDPOINT((USBx),
                               (bEpNum)) & USB_EP_T_MASK) |
                               (wType) | USB_EP_CTR_TX | USB_EP_CTR_RX)));
}

/**
  * @brief  gets the type in the endpoint register(bits EP_TYPE[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval Endpoint Type
  */
static inline uint16_t
USBD_PCD_GET_EPTYPE(USB_TypeDef* USBx, uint8_t bEpNum) {
  return (USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EP_T_FIELD);
}

/**
  * @brief free buffer used from the application realizing it to the line
  *         toggles bit SW_BUF in the double buffered endpoint register
  * @param USBx USB device.
  * @param   bEpNum, bDir
  * @retval None
  */
static inline void
USBD_PCD_FREE_USER_BUFFER(USB_TypeDef* USBx, uint8_t bEpNum, uint8_t bDir) {
  do {
    if ((bDir) == 0U)
    {
      /* OUT double buffered endpoint */
      USBD_PCD_TX_DTOG((USBx), (bEpNum));
    }
    else if ((bDir) == 1U)
    {
      /* IN double buffered endpoint */
      USBD_PCD_RX_DTOG((USBx), (bEpNum));
    }
  } while(0);
}

/**
  * @brief  sets the status for tx transfer (bits STAT_TX[1:0]).
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wState new state
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_TX_STATUS(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t wState) {
  do {
    uint16_t _wRegVal;
    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPTX_DTOGMASK;
    /* toggle first bit ? */
    if ((USB_EPTX_DTOG1 & (wState))!= 0U)
    {
      _wRegVal ^= USB_EPTX_DTOG1;
    }
    /* toggle second bit ?  */
    if ((USB_EPTX_DTOG2 & (wState))!= 0U)
    {
      _wRegVal ^= USB_EPTX_DTOG2;
    }
    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  } while(0); /* USBD_PCD_SET_EP_TX_STATUS */
}

/**
  * @brief  sets the status for rx transfer (bits STAT_TX[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wState new state
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_RX_STATUS(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t wState) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPRX_DTOGMASK;
    /* toggle first bit ? */
    if ((USB_EPRX_DTOG1 & (wState))!= 0U)
    {
      _wRegVal ^= USB_EPRX_DTOG1;
    }
    /* toggle second bit ? */
    if ((USB_EPRX_DTOG2 & (wState))!= 0U)
    {
      _wRegVal ^= USB_EPRX_DTOG2;
    }
    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  } while(0); /* USBD_PCD_SET_EP_RX_STATUS */
}

/**
  * @brief  sets the status for rx & tx (bits STAT_TX[1:0] & STAT_RX[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wStaterx new state.
  * @param  wStatetx new state.
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_TXRX_STATUS(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t wStaterx, uint16_t wStatetx) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & (USB_EPRX_DTOGMASK | USB_EPTX_STAT);
    /* toggle first bit ? */
    if ((USB_EPRX_DTOG1 & (wStaterx))!= 0U)
    {
      _wRegVal ^= USB_EPRX_DTOG1;
    }
    /* toggle second bit ? */
    if ((USB_EPRX_DTOG2 & (wStaterx))!= 0U)
    {
      _wRegVal ^= USB_EPRX_DTOG2;
    }
    /* toggle first bit ? */
    if ((USB_EPTX_DTOG1 & (wStatetx))!= 0U)
    {
      _wRegVal ^= USB_EPTX_DTOG1;
    }
    /* toggle second bit ?  */
    if ((USB_EPTX_DTOG2 & (wStatetx))!= 0U)
    {
      _wRegVal ^= USB_EPTX_DTOG2;
    }

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  } while(0); /* USBD_PCD_SET_EP_TXRX_STATUS */
}

/**
  * @brief  gets the status for tx/rx transfer (bits STAT_TX[1:0]
  *         /STAT_RX[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval status
  */
static inline uint16_t
USBD_PCD_GET_EP_TX_STATUS(USB_TypeDef* USBx, uint8_t bEpNum) {
  return ((uint16_t)USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPTX_STAT);
}

static inline uint16_t
USBD_PCD_GET_EP_RX_STATUS(USB_TypeDef* USBx, uint8_t bEpNum) {
  return ((uint16_t)USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPRX_STAT);
}

/**
  * @brief  sets directly the VALID tx/rx-status into the endpoint register
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_TX_VALID(USB_TypeDef* USBx, uint8_t bEpNum) {
  (USBD_PCD_SET_EP_TX_STATUS((USBx), (bEpNum), USB_EP_TX_VALID));
}

static inline void
USBD_PCD_SET_EP_RX_VALID(USB_TypeDef* USBx, uint8_t bEpNum) {
  (USBD_PCD_SET_EP_RX_STATUS((USBx), (bEpNum), USB_EP_RX_VALID));
}

/**
  * @brief  checks stall condition in an endpoint.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval TRUE = endpoint in stall condition.
  */
static inline void
USBD_PCD_GET_EP_TX_STALL_STATUS(USB_TypeDef* USBx, uint8_t bEpNum) {
  (USBD_PCD_GET_EP_TX_STATUS((USBx), (bEpNum)) == USB_EP_TX_STALL);
}

static inline void
USBD_PCD_GET_EP_RX_STALL_STATUS(USB_TypeDef* USBx, uint8_t bEpNum) {
  (USBD_PCD_GET_EP_RX_STATUS((USBx), (bEpNum)) == USB_EP_RX_STALL);
}

/**
  * @brief  set & clear EP_KIND bit.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_KIND(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK;

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_KIND));
  } while(0); /* USBD_PCD_SET_EP_KIND */
}

static inline void
USBD_PCD_CLEAR_EP_KIND(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPKIND_MASK;

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  } while(0); /* USBD_PCD_CLEAR_EP_KIND */
}

/**
  * @brief  Sets/clears directly STATUS_OUT bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
static inline void
USBD_PCD_SET_OUT_STATUS(USB_TypeDef* USBx, uint8_t bEpNum) {
  USBD_PCD_SET_EP_KIND((USBx), (bEpNum));
}

static inline void
USBD_PCD_CLEAR_OUT_STATUS(USB_TypeDef* USBx, uint8_t bEpNum) {
  USBD_PCD_CLEAR_EP_KIND((USBx), (bEpNum));
}

/**
  * @brief  Sets/clears directly EP_KIND bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
static inline void
USBD_PCD_SET_BULK_EP_DBUF(USB_TypeDef* USBx, uint8_t bEpNum) {
  USBD_PCD_SET_EP_KIND((USBx), (bEpNum));
}

static inline void
USBD_PCD_CLEAR_BULK_EP_DBUF(USB_TypeDef* USBx, uint8_t bEpNum) {
  USBD_PCD_CLEAR_EP_KIND((USBx), (bEpNum));
}

/**
  * @brief  Clears bit CTR_RX / CTR_TX in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */

static inline void
USBD_PCD_CLEAR_RX_EP_CTR(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & (0x7FFFU & USB_EPREG_MASK);

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_TX));
  } while(0); /* USBD_PCD_CLEAR_RX_EP_CTR */
}

static inline void
USBD_PCD_CLEAR_TX_EP_CTR(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & (0xFF7FU & USB_EPREG_MASK);

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX));
  } while(0); /* USBD_PCD_CLEAR_TX_EP_CTR */
}

/**
  * @brief  Toggles DTOG_RX / DTOG_TX bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
static inline void
USBD_PCD_RX_DTOG(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wEPVal;

    _wEPVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK;

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wEPVal | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_RX));
  } while(0); /* USBD_PCD_RX_DTOG */
}

static inline void
USBD_PCD_TX_DTOG(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wEPVal;

    _wEPVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK;

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wEPVal | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_TX));
  } while(0); /* USBD_PCD_TX_DTOG */
}

static inline void
USBD_PCD_CLEAR_RX_DTOG(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum));

    if ((_wRegVal & USB_EP_DTOG_RX) != 0U)
    {
      USBD_PCD_RX_DTOG((USBx), (bEpNum));
    }
  } while(0); /* USBD_PCD_CLEAR_RX_DTOG */
}

static inline void
USBD_PCD_CLEAR_TX_DTOG(USB_TypeDef* USBx, uint8_t bEpNum) {
  do {
    uint16_t _wRegVal;

    _wRegVal = USBD_PCD_GET_ENDPOINT((USBx), (bEpNum));

    if ((_wRegVal & USB_EP_DTOG_TX) != 0U)
    {
      USBD_PCD_TX_DTOG((USBx), (bEpNum));
    }
  } while(0); /* USBD_PCD_CLEAR_TX_DTOG */
}

/**
  * @brief  Sets address in an endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  bAddr Address.
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_ADDRESS(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t bAddr) {
  do {
    uint16_t _wRegVal;

    _wRegVal = (USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK) | (bAddr);

    USBD_PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  } while(0); /* USBD_PCD_SET_EP_ADDRESS */
}

/**
  * @brief  Gets address in an endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
static inline void
USBD_PCD_GET_EP_ADDRESS(USB_TypeDef* USBx, uint8_t bEpNum) {
  ((uint8_t)(USBD_PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPADDR_FIELD));
}

static inline uint16_t *
USBD_PCD_EP_TX_CNT(USB_TypeDef* USBx, uint8_t bEpNum) {
  return ((uint16_t *)((((uint32_t)(USBx)->BTABLE +
         ((uint32_t)(bEpNum) * 8U) + 2U) *
         PMA_ACCESS) + ((uint32_t)(USBx) + 0x400U)));
}

static inline uint16_t *
USBD_PCD_EP_RX_CNT(USB_TypeDef* USBx, uint8_t bEpNum) {
  return ((uint16_t *)((((uint32_t)(USBx)->BTABLE +
         ((uint32_t)(bEpNum) * 8U) + 6U) *
         PMA_ACCESS) + ((uint32_t)(USBx) + 0x400U)));
}

/**
  * @brief  sets address of the tx/rx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wAddr address to be set (must be word aligned).
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_TX_ADDRESS(USB_TypeDef* USBx, uint8_t bEpNum, uint32_t wAddr) {
  do {
    __IO uint16_t *_wRegVal;
    uint32_t _wRegBase = (uint32_t)USBx;

    _wRegBase += (uint32_t)(USBx)->BTABLE;
    _wRegVal = (__IO uint16_t *)(_wRegBase + 0x400U + (((uint32_t)(bEpNum) * 8U) * PMA_ACCESS));
    *_wRegVal = ((wAddr) >> 1) << 1;
  } while(0); /* USBD_PCD_SET_EP_TX_ADDRESS */
}

static inline void
USBD_PCD_SET_EP_RX_ADDRESS(USB_TypeDef* USBx, uint8_t bEpNum, uint32_t wAddr) {
  do {
    __IO uint16_t *_wRegVal;
    uint32_t _wRegBase = (uint32_t)USBx;

    _wRegBase += (uint32_t)(USBx)->BTABLE;
    _wRegVal = (__IO uint16_t *)(_wRegBase + 0x400U + ((((uint32_t)(bEpNum) * 8U) + 4U) * PMA_ACCESS));
    *_wRegVal = ((wAddr) >> 1) << 1;
  } while(0); /* USBD_PCD_SET_EP_RX_ADDRESS */
}

/**
  * @brief  Gets address of the tx/rx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval address of the buffer.
  */
// static inline uint16_t
// USBD_PCD_GET_EP_TX_ADDRESS(USB_TypeDef* USBx, uint8_t bEpNum) {
//   return ((uint16_t)*USBD_PCD_EP_TX_ADDRESS((USBx), (bEpNum)));
// }
//
// static inline uint16_t
// USBD_PCD_GET_EP_RX_ADDRESS(USB_TypeDef* USBx, uint8_t bEpNum) {
//   return ((uint16_t)*USBD_PCD_EP_RX_ADDRESS((USBx), (bEpNum)));
// }

/**
  * @brief  Sets counter of rx buffer with no. of blocks.
  * @param  pdwReg Register pointer
  * @param  wCount Counter.
  * @param  wNBlocks no. of Blocks.
  * @retval None
  */
static inline void
USBD_PCD_CALC_BLK32(uint16_t* pdwReg, uint16_t wCount, uint32_t wNBlocks) {
  do {
    (wNBlocks) = (wCount) >> 5;
    if (((wCount) & 0x1fU) == 0U)
    {
      (wNBlocks)--;
    }
    *(pdwReg) |= (uint16_t)(((wNBlocks) << 10) | USB_CNTRX_BLSIZE);
  } while(0); /* USBD_PCD_CALC_BLK32 */
}

static inline void
USBD_PCD_CALC_BLK2(uint16_t* pdwReg, uint16_t wCount, uint32_t wNBlocks) {
  do {
    (wNBlocks) = (wCount) >> 1;
    if (((wCount) & 0x1U) != 0U)
    {
      (wNBlocks)++;
    }
    *(pdwReg) |= (uint16_t)((wNBlocks) << 10);
  } while(0); /* USBD_PCD_CALC_BLK2 */
}

static inline void
USBD_PCD_SET_EP_CNT_RX_REG(uint16_t* pdwReg, uint16_t wCount) {
  do {
    uint32_t wNBlocks;

    *(pdwReg) &= 0x3FFU;

    if ((wCount) > 62U)
    {
      USBD_PCD_CALC_BLK32((pdwReg), (wCount), wNBlocks);
    }
    else
    {
      if ((wCount) == 0U)
      {
        *(pdwReg) |= USB_CNTRX_BLSIZE;
      }
      else
      {
        USBD_PCD_CALC_BLK2((pdwReg), (wCount), wNBlocks);
      }
    }
  } while(0); /* USBD_PCD_SET_EP_CNT_RX_REG */
}

static inline void
USBD_PCD_SET_EP_RX_DBUF0_CNT(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t wCount) {
  do {
    uint32_t _wRegBase = (uint32_t)(USBx);
    __IO uint16_t *pdwReg;

    _wRegBase += (uint32_t)(USBx)->BTABLE;
    pdwReg = (__IO uint16_t *)(_wRegBase + 0x400U +
    ((((uint32_t)(bEpNum) * 8U) + 2U) * PMA_ACCESS));
    USBD_PCD_SET_EP_CNT_RX_REG(pdwReg, (wCount));
  } while(0);
}

/**
  * @brief  sets counter for the tx/rx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wCount Counter value.
  * @retval None
  */
static inline void
USBD_PCD_SET_EP_TX_CNT(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t wCount) {
  do {
    uint32_t _wRegBase = (uint32_t)(USBx);
    __IO uint16_t *_wRegVal;

    _wRegBase += (uint32_t)(USBx)->BTABLE;
    _wRegVal = (__IO uint16_t *)(_wRegBase + 0x400U +
    ((((uint32_t)(bEpNum) * 8U) + 2U) * PMA_ACCESS));
    *_wRegVal = (uint16_t)(wCount);
  } while(0);
}

static inline void
USBD_PCD_SET_EP_RX_CNT(USB_TypeDef* USBx, uint8_t bEpNum, uint16_t wCount) {
  do {
    uint32_t _wRegBase = (uint32_t)(USBx);
    __IO uint16_t *_wRegVal;

    _wRegBase += (uint32_t)(USBx)->BTABLE;
    _wRegVal = (__IO uint16_t *)(_wRegBase + 0x400U +
    ((((uint32_t)(bEpNum) * 8U) + 6U) * PMA_ACCESS));
    USBD_PCD_SET_EP_CNT_RX_REG(_wRegVal, (wCount));
  } while(0);
}

/**
  * @brief  gets counter of the tx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval Counter value
  */
static inline uint32_t
USBD_PCD_GET_EP_TX_CNT(USB_TypeDef* USBx, uint8_t bEpNum) {
  return ((uint32_t)(*USBD_PCD_EP_TX_CNT((USBx), (bEpNum))) & 0x3ffU);
}

static inline uint32_t
USBD_PCD_GET_EP_RX_CNT(USB_TypeDef* USBx, uint8_t bEpNum) {
  return ((uint32_t)(*USBD_PCD_EP_RX_CNT((USBx), (bEpNum))) & 0x3ffU);
}

#endif /* defined (USB) */

#ifdef __cplusplus
}
#endif

#endif /* STM32F1xx_USBD_PCD_H */
