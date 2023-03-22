/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usb_vcp/usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

typedef struct {
	uint16_t firstChar;
	uint16_t lastChar;

	uint8_t noMessages;
	uint8_t empty;

	int16_t nextMessageEnd;

	uint8_t buf[COMMAND_BUF_SIZE];
} ComMsgBuf;

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

ComMsgBuf msgBuf =
{
  0,  // firstChar
  0,  // lastChar
  1,  // noMessages
  1,  // empty
  -1, // nextMessageEnd
  { '\0' } // buf
};

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

static uint8_t Com_Append_Data(uint8_t *buf, uint16_t len);
static uint16_t Get_Buffer_Free_Space();
static uint16_t Get_Cyclic_Distance(uint16_t start, uint16_t end, uint16_t bufSize);

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);

  if (Com_Append_Data(Buf, *Len) != BUF_OK) {
	  Error_Handler();
  }
  
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

static uint8_t Com_Append_Data(uint8_t *buf, uint16_t len)
{
  BufStatusTypeDef status = BUF_OK;

  if (Get_Buffer_Free_Space() < len) {
	  return BUF_FULL;
  }

  if (msgBuf.lastChar + len >= COMMAND_BUF_SIZE) {
	  uint16_t symbolsToCycle = COMMAND_BUF_SIZE - msgBuf.lastChar;
	  strncpy((char*)&msgBuf.buf[msgBuf.lastChar], (char*)&buf[0], symbolsToCycle);
	  strncpy((char*)&msgBuf.buf[0], (char*)&buf[symbolsToCycle], len - symbolsToCycle);

	msgBuf.lastChar = msgBuf.lastChar + len - COMMAND_BUF_SIZE;
  } else {
	  strncpy((char*)&msgBuf.buf[msgBuf.lastChar], (char*)&buf[0], len);

	  msgBuf.lastChar = msgBuf.lastChar + len;
  }

  if (len > 0) {
	  msgBuf.noMessages = 0;
	  msgBuf.empty = 0;
  }

  return status;
}

static uint16_t Get_Buffer_Free_Space()
{
  uint16_t result;
  if (msgBuf.lastChar < msgBuf.firstChar && !msgBuf.empty) {
	  result = msgBuf.firstChar - msgBuf.lastChar;
  } else {
	  result = COMMAND_BUF_SIZE - (msgBuf.lastChar - msgBuf.firstChar);
  }
  return result;
}

static uint16_t Get_Cyclic_Distance(uint16_t start, uint16_t end, uint16_t bufSize)
{
  uint16_t result;
  if (end <= start) {
	  result = COMMAND_BUF_SIZE - (start - end);
  } else {
	  result = end - start;
  }
  return result;
}

uint8_t Com_Msg_Available()
{
  __disable_irq();
  uint8_t result = 0;
  uint16_t cur = msgBuf.firstChar;
  if (!msgBuf.empty && !msgBuf.noMessages && (msgBuf.nextMessageEnd == -1)) {
    do {
      if (msgBuf.buf[cur] == COMMAND_DELIMITER_SYMB) {
      result = 1;
      }

      ++cur;

      if (cur >= COMMAND_BUF_SIZE) {
      cur = 0;
      }
    }
	  while ((cur != msgBuf.lastChar) && (result == 0));
  }

  if (!result) {
    msgBuf.noMessages = 1;
    msgBuf.nextMessageEnd = -1;
  } else {
    msgBuf.noMessages = 0;
    msgBuf.nextMessageEnd = cur;
  }

  __enable_irq();
  return result || msgBuf.nextMessageEnd != -1;
}

uint8_t Com_Read_Msg(uint8_t *buf, uint16_t bufLen, uint16_t *msgLen)
{
  BufStatusTypeDef result = BUF_OK;
  if (!Com_Msg_Available()) {
    return BUF_NOMSGAVAIL;
  }

  __disable_irq();
  uint16_t len = Get_Cyclic_Distance(msgBuf.firstChar, msgBuf.nextMessageEnd, COMMAND_BUF_SIZE);

  if (bufLen < len) {
	  return BUF_NOTENOUGHSPACE;
  }

  if (msgBuf.firstChar + len >= COMMAND_BUF_SIZE) {
    uint16_t symbolsToCycle = COMMAND_BUF_SIZE - msgBuf.firstChar;
    strncpy((char*)&buf[0], (char*)&msgBuf.buf[msgBuf.firstChar], symbolsToCycle);
    strncpy((char*)&buf[symbolsToCycle], (char*)&msgBuf.buf[0], len - symbolsToCycle);

    msgBuf.firstChar += len - COMMAND_BUF_SIZE;
  } else {
	  strncpy((char*)&buf[0], (char*)&msgBuf.buf[msgBuf.firstChar], len);

	  msgBuf.firstChar += len;
  }

  msgBuf.nextMessageEnd = -1;

  if (msgBuf.firstChar == msgBuf.lastChar) {
	  msgBuf.empty = 1;
	  msgBuf.noMessages = 1;
  }

  *msgLen = len;

  __enable_irq();
  return result;
}

void Com_Buf_Reset()
{
	__disable_irq();
	msgBuf.empty = 1;
	msgBuf.firstChar = 0;
	msgBuf.lastChar = 0;
	msgBuf.noMessages = 1;
	msgBuf.nextMessageEnd = -1;
	__enable_irq();
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
