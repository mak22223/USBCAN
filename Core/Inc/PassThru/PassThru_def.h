/*
 * PassThru_if.h
 *
 *  Created on: 23 дек. 2022 г.
 *      Author: mak22
 */

#ifndef INC_PASSTHRU_PASSTHRU_DEF_H_
#define INC_PASSTHRU_PASSTHRU_DEF_H_

/*
 * Restrictions:
 * - Cannot receive filter parameters bigger than 64 bytes
 * - Without EXTENDED_J2534_SUPPORT define IOCTL FAST_INIT cannot be supported
 */

/* ---------------- PassThruInterface includes ------------------- */

#include "stdint.h"


#define MAX_CONFIG_PARAMS 15
#define MAX_BYTES_SARRAY MAX_CONFIG_PARAMS * 2 * sizeof(uint32_t)

/* ------------- PassThruInterface enums declaration ------------------- */

typedef enum {
  NO_COMMAND = 0U,
  CONNECT,
  DISCONNECT,
  READ_MSGS,
  WRITE_MSGS,
  START_PERIODIC_MSG,
  STOP_PERIODIC_MSG,
  START_MSG_FILTER,
  STOP_MSG_FILTER,
  SET_PROGRAMMING_VOLTAGE,
  READ_VERSION,
  GET_LAST_ERROR,
  IOCTL
} PassThruCommand;

typedef enum {
  GET_CONFIG = 0x1U,
  SET_CONFIG = 0x2U,
  READ_VBATT = 0x3U,
  FIVE_BAUD_INIT = 0x4U,
  FAST_INIT = 0x5U,
  CLEAR_TX_BUFFER = 0x7U,
  CLEAR_RX_BUFFER = 0x8U,
  CLEAR_PERIODIC_MSGS = 0x9U,
  CLEAR_MSG_FILTERS = 0xAU,
  CLEAR_FUNCT_MSG_LOOKUP_TABLE = 0xBU,
  ADD_TO_FUNCT_MSG_LOOKUP_TABLE = 0xCU,
  DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE = 0xDU,
  READ_PROG_VOLTAGE = 0xEU
} PassThruIoctlId;

typedef enum {
  STATUS_NOERROR = 0U,
  ERR_NOT_SUPPORTED,
  ERR_INVALID_CHANNEL_ID,
  ERR_INVALID_PROTOCOL_ID,
  ERR_NULLPARAMETER,
  ERR_INVALID_IOCTL_VALUE,
  ERR_INVALID_FLAGS,
  ERR_FAILED,
  ERR_DEVICE_NOT_CONNECTED,
  ERR_TIMEOUT,
  ERR_INVALID_MSG,
  ERR_INVALID_TIME_INTERVAL,
  ERR_EXCEEDED_LIMIT,
  ERR_INVALID_MSG_ID,
  ERR_INVALID_ERROR_ID,
  ERR_INVAILD_IOCTL_ID,
  ERR_BUFFER_EMPTY,
  ERR_BUFFER_FULL,
  ERR_BUFFER_OVERFLOW,
  ERR_PIN_INVALID,
  ERR_CHANNEL_IN_USE,
  ERR_MSG_PROTOCOL_ID
} PassThruError;

typedef enum {
  J1850VPW = 1U,
  J1850PWM,
  ISO9141,
  ISO14230,
  CAN,
  ISO15765,
  SCI_A_ENGINE,
  SCI_A_TRANS,
  SCI_B_ENGINE,
  SCI_B_TRANS,
  MS_CAN = 0x10000U
} PassThruProtocolId;

/* ------------- PassThruInterface structs declaration ------------------- */

typedef struct {
  uint32_t parameter;
  uint32_t value;
} PassThruSConfig;

typedef struct {
  uint32_t ProtocolID;
  uint32_t RxStatus;
  uint32_t TxFlags;
  uint32_t Timestamp;
  uint32_t DataSize;
  uint32_t ExtraDataIndex;
  uint8_t Data[4128];
} PassThruMessage;

/// Shortened structure to reduce memory usage
typedef struct {
  uint32_t ProtocolID;
  uint32_t RxStatus;
  uint32_t TxFlags;
  uint32_t Timestamp;
  uint32_t DataSize;
  uint32_t ExtraDataIndex;
  uint8_t Data[64];
} PassThruMessageFilter;

typedef union {
  struct SCONFIG_LIST {
    uint32_t numOfParams;
    PassThruSConfig params[MAX_CONFIG_PARAMS * 2];
  } sConfigList;

  struct SBYTE_ARRAY {
    uint32_t numOfBytes;
    uint8_t bytes[MAX_BYTES_SARRAY];
  } sByteArray;

#ifdef EXTENDED_J2534_SUPPORT
  PassThruMessage msg;
#endif
} PassThruIoctl;

typedef union {
  struct {
    uint32_t protocolId;
    uint32_t flags;
  } Connect;

  struct  {
    uint32_t channelId;
  } Disconnect;

  struct  {
    uint32_t channelId;
    uint32_t numMsgs;
    uint32_t timeout;
  } ReadMsgs;

  struct {
    uint32_t channelId;
    uint32_t timeout;
    PassThruMessage msg;
  } WriteMsgs;

  struct {
    uint32_t channelId;
    uint32_t interval;
    PassThruMessage msg;
  } StartPeriodicMsg;

  struct {
    uint32_t channelId;
    uint32_t msgId;
  } StopPeriodicMsg;

  struct {
    uint32_t channelId;
    uint32_t filterType;
    PassThruMessageFilter mask;
    PassThruMessageFilter pattern;
    PassThruMessageFilter flowControl;
  } StartMsgFilter;

  struct {
    uint32_t channelId;
    uint32_t msgId;
  } StopMsgFilter;

  struct {
    uint32_t pinNumber;
    uint32_t voltage;
  } SetProgrammingVoltage;

  struct {

  } ReadVersion;

  struct {

  } GetLastError;

  struct {
    uint32_t channelId;
    uint32_t ioctlId;
    PassThruIoctl ioctl;
  } IOCTL;
} PassThruParams;

typedef struct {
  uint32_t errorCode;

  union {
    struct {
      uint32_t channelId;
    } Connect;

    struct {

    } Disconnect;

    struct {
      PassThruMessage msg;
    } ReadMsgs;

    struct {

    } WriteMsgs;

    struct {
      uint32_t msgId;
    } StartPeriodicMsg;

    struct {

    } StopPeriodicMsg;

    struct {
      uint32_t msgId;
    } StartMsgFilter;

    struct {

    } StopMsgFilter;

    struct {

    } SetProgrammingVoltage;

    struct {
      uint8_t *string;
    } ReadVersion;

    struct {
      uint8_t *string;
    } GetLastError;

    struct {
      uint32_t ioctlId;
      void *ptr;
    } IOCTL;
  };

} PassThruAnswer;

#endif /* INC_PASSTHRU_PASSTHRU_DEF_H_ */
