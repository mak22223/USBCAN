/*
 * VCPCommParser.c
 *
 *  Created on: 23 дек. 2022 г.
 *      Author: mak22
 */

/*
 * TODO: - команды без пробела в конце, не обрабатываются
 */

#include "usb_vcp/VCPCommParser.h"

/* ---------- VCP command parser private enum ----------- */

typedef enum {
  PARSE_ERROR = 0U,
  PARSE_COMMA,
  PARSE_END
} ParseStatus;

/* ---------- VCP command parser private variables ----------- */

#define COMMAND_COUNT 12
#define MAX_COMMAND_LEN 18


const char commands[COMMAND_COUNT][MAX_COMMAND_LEN] = {
    "connect",
    "disconnect",
    "readmsgs",
    "writemsgs",
    "startperiodicmsgs",
    "stopperiodicmsgs",
    "startmsgfilter",
    "stopmsgfilter",
    "setprogvoltage",
    "readversion",
    "getlasterror",
    "ioctl"
};

const char error_string[22][27] = {
    "STATUS_NOERROR\r",
    "ERR_NOT_SUPPORTED\r",
    "ERR_INVALID_CHANNEL_ID\r",
    "ERR_INVALID_PROTOCOL_ID\r",
    "ERR_NULLPARAMETER\r",
    "ERR_INVALID_IOCTL_VALUE\r",
    "ERR_INVALID_FLAGS\r",
    "ERR_FAILED\r",
    "ERR_DEVICE_NOT_CONNECTED\r",
    "ERR_TIMEOUT\r",
    "ERR_INVALID_MSG\r",
    "ERR_INVALID_TIME_INTERVAL\r",
    "ERR_EXCEEDED_LIMIT\r",
    "ERR_INVALID_MSG_ID\r",
    "ERR_INVALID_ERROR_ID\r",
    "ERR_INVAILD_IOCTL_ID\r",
    "ERR_BUFFER_EMPTY\r",
    "ERR_BUFFER_FULL\r",
    "ERR_BUFFER_OVERFLOW\r",
    "ERR_PIN_INVALID\r",
    "ERR_CHANNEL_IN_USE\r",
    "ERR_MSG_PROTOCOL_ID\r"
};

uint8_t question[] = { '?', COMMAND_DELIMITER_SYMB };


/* ---------- VCP command parser private function declaration ----------- */

static uint8_t init(void);
static uint8_t deinit(void);
static uint8_t receiveCmd(uint8_t *cmd, PassThruParams *params);
static uint8_t sendAnswer(uint8_t *cmd, PassThruAnswer *ans);

static uint8_t parseInput(
    uint8_t *cmdBuf,
    uint8_t *cmd,
    PassThruParams *params
);

static uint8_t determineCommand(uint8_t *cmdBuf, uint8_t *cmd);
static uint8_t parseParams(uint8_t *cmdBuf, uint8_t cmd, PassThruParams *params);

static uint8_t isCommand(uint8_t *cmdBuf, uint16_t cmdLen, uint8_t *cmd);
static uint8_t parseMsg(uint8_t **iter, PassThruMessage *result);
static uint8_t parseInt(uint8_t **iter, uint32_t *result);
static uint8_t parseArray(uint8_t **iter, uint8_t *result);
static void prepareIntToSend(uint32_t num, uint8_t *buf);
static void prepareMsgToSend(PassThruMessage *msg, uint8_t *buf, uint16_t *len);
static void sendErrorCode(uint32_t code);
static void sendVcpData(uint8_t *buf, uint16_t len);

void VCP_getInterface(PassThruComm_ItfTypeDef *interface)
{
  PassThruComm_ItfTypeDef _interface = {
    init,
    deinit,
    receiveCmd,
    sendAnswer
  };

  *interface = _interface;
  return;
}

static uint8_t init(void)
{
  uint8_t result = IF_OK;

  Com_Buf_Reset();

  return result;
}

static uint8_t deinit(void)
{
  uint8_t result = IF_OK;

  Com_Buf_Reset();

  return result;
}

static uint8_t receiveCmd(uint8_t *cmd, PassThruParams *params)
{
  uint8_t result = IF_OK;

  uint8_t cmdBuf[COMMAND_BUF_SIZE + 1];
  uint16_t msgLen = 0;
  switch (Com_Read_Msg(cmdBuf, COMMAND_BUF_SIZE, &msgLen)) {
    case BUF_NOMSGAVAIL:
      *cmd = NO_COMMAND;
      break;

    case BUF_OK:
      cmdBuf[msgLen] = '\0';
      if (parseInput(cmdBuf, cmd, params) != IF_OK) {
        sendVcpData(cmdBuf, msgLen);
        sendVcpData(question, sizeof(question));
      }

      break;

    case BUF_NOTENOUGHSPACE:
      result = IF_ERROR;
      Error_Handler();
      break;

    default:
      result = IF_ERROR;
      Error_Handler();
      break;
  }

  return result;
}

static uint8_t sendAnswer(uint8_t *cmd, PassThruAnswer *ans)
{
  uint8_t result = IF_OK;
  if (ans->errorCode != STATUS_NOERROR) {
    sendErrorCode(ans->errorCode);
    return result;
  }

  /// TODO: заменить на больший буфер
  uint8_t sendBuf[200];
  uint16_t strLen = 0;

  switch (*cmd) {
    case CONNECT:
      prepareIntToSend(ans->Connect.channelId, sendBuf);
      sendVcpData(sendBuf, 8 + 1);
      break;

    case DISCONNECT:
      sendErrorCode(ans->errorCode);
      break;

    case READ_MSGS:
      prepareMsgToSend(&ans->ReadMsgs.msg, sendBuf, &strLen);
      sendVcpData(sendBuf, strLen);
      break;

    case WRITE_MSGS:

      break;

    case START_PERIODIC_MSG:

      break;

    case STOP_PERIODIC_MSG:

      break;

    case START_MSG_FILTER:

      break;

    case STOP_MSG_FILTER:

      break;

    case SET_PROGRAMMING_VOLTAGE:

      break;

    case READ_VERSION:

      break;

    case GET_LAST_ERROR:

      break;

    case IOCTL:

      break;

    default:

      break;
  }

  return result;
}

static uint8_t parseInput(
    uint8_t *cmdBuf,
    uint8_t *cmd,
    PassThruParams *params)
{
  uint8_t status = IF_OK;
  *cmd = NO_COMMAND;

  if (determineCommand(cmdBuf, cmd) != IF_OK ||
      parseParams(cmdBuf, *cmd, params) != IF_OK)
  {
    *cmd = NO_COMMAND;
    status = IF_ERROR;
    return status;
  }

  return status;
}

static uint8_t determineCommand(uint8_t *cmdBuf, uint8_t *cmd)
{
  uint8_t status = IF_OK;

  uint16_t cmdLen = 0;
  for (cmdLen = 0; (cmdBuf[cmdLen] != COMMAND_DELIMITER_SYMB) &&
         (cmdBuf[cmdLen] != ' '); ++cmdLen);

  if ((cmdBuf[cmdLen] == COMMAND_DELIMITER_SYMB) || isCommand(cmdBuf, cmdLen, cmd) != IF_OK) {
    status = IF_ERROR;
  }

  return status;
}

static uint8_t parseParams(uint8_t *cmdBuf, uint8_t cmd, PassThruParams *params)
{
  uint8_t status = IF_OK;
  uint8_t *iter = cmdBuf;
  for (; *(iter++) != ' '; );

  uint8_t parseOk = 0;
  switch (cmd) {
    case CONNECT:
      parseOk = (parseInt(&iter, &params->Connect.protocolId) == PARSE_COMMA) &&
        (parseInt(&iter, &params->Connect.flags) == PARSE_END);
      break;

    case DISCONNECT:
      parseOk = parseInt(&iter, &params->Disconnect.channelId) == PARSE_END;
      break;

    case READ_MSGS:
      parseOk = (parseInt(&iter, &params->ReadMsgs.channelId) == PARSE_COMMA) &&
        (parseInt(&iter, &params->ReadMsgs.numMsgs) == PARSE_COMMA) &&
        (parseInt(&iter, &params->ReadMsgs.timeout) == PARSE_END);
      break;

    case WRITE_MSGS:
      parseOk = (parseInt(&iter, &params->WriteMsgs.channelId) == PARSE_COMMA) &&
        (parseMsg(&iter, &params->WriteMsgs.msg) == PARSE_END);
      break;

    case START_PERIODIC_MSG:
      parseOk = (parseInt(&iter, &params->StartPeriodicMsg.channelId) == PARSE_COMMA) &&
        (parseInt(&iter, &params->StartPeriodicMsg.interval) == PARSE_COMMA) &&
        (parseMsg(&iter, &params->StartPeriodicMsg.msg) == PARSE_END);
      break;

    case STOP_PERIODIC_MSG:
      parseOk = (parseInt(&iter, &params->StopPeriodicMsg.channelId) == PARSE_COMMA) &&
        (parseInt(&iter, &params->StopPeriodicMsg.msgId) == PARSE_END);
      break;

    case START_MSG_FILTER:

      break;

    case STOP_MSG_FILTER:

      break;

    case SET_PROGRAMMING_VOLTAGE:

      break;

    case READ_VERSION:
      parseOk = 1;
      break;

    case GET_LAST_ERROR:
      parseOk = 1;
      break;

    case IOCTL:
      parseOk = (parseInt(&iter, &params->IOCTL.channelId) == PARSE_COMMA);

      uint8_t ioctlIdParseStatus;
      if (parseOk) {
        ioctlIdParseStatus = parseInt(&iter, &params->IOCTL.ioctlId);
      }
      parseOk = parseOk && ioctlIdParseStatus;

      if (parseOk) {
        switch (params->IOCTL.ioctlId) {
          case GET_CONFIG:
            if (ioctlIdParseStatus != PARSE_COMMA) {
              parseOk = 0;
              break;
            }

            parseOk = parseInt(&iter, &params->IOCTL.ioctl.sConfigList.numOfParams) == PARSE_COMMA;

            if (parseOk && params->IOCTL.ioctl.sConfigList.numOfParams < MAX_CONFIG_PARAMS) {
              uint8_t i = 0;
              while (parseOk && (i < params->IOCTL.ioctl.sConfigList.numOfParams - 1)) {
                parseOk = parseInt(&iter, &params->IOCTL.ioctl.sConfigList.params[i++].parameter) == PARSE_COMMA;
              }
              if (parseOk) {
                parseOk = parseInt(&iter, &params->IOCTL.ioctl.sConfigList.params[i].parameter) == PARSE_END;
              }
            }
            break;

          case SET_CONFIG:
            if (ioctlIdParseStatus != PARSE_COMMA) {
              parseOk = 0;
              break;
            }

            parseOk = parseInt(&iter, &params->IOCTL.ioctl.sConfigList.numOfParams) == PARSE_COMMA;

            if (parseOk && params->IOCTL.ioctl.sConfigList.numOfParams < MAX_CONFIG_PARAMS) {
              uint8_t i = 0;
              while (parseOk && (i < (params->IOCTL.ioctl.sConfigList.numOfParams - 1))) {
                parseOk = (parseInt(&iter, &params->IOCTL.ioctl.sConfigList.params[i].parameter) == PARSE_COMMA) &&
                              (parseInt(&iter, &params->IOCTL.ioctl.sConfigList.params[i++].value) == PARSE_COMMA);
              }
              if (parseOk) {
                parseOk = (parseInt(&iter, &params->IOCTL.ioctl.sConfigList.params[i].parameter) == PARSE_COMMA) &&
                              (parseInt(&iter, &params->IOCTL.ioctl.sConfigList.params[i].value) == PARSE_END);
              }
            }
            break;

          case READ_VBATT:
            if (ioctlIdParseStatus != PARSE_END) {
              parseOk = 0;
              break;
            }

            parseOk = 1;
            break;

          case FIVE_BAUD_INIT:
            parseOk = 1; // stub as not supported
            break;

          case FAST_INIT:
            parseOk = 1; // stub as not supported
            break;

          case CLEAR_TX_BUFFER:
            if (ioctlIdParseStatus != PARSE_END) {
              parseOk = 0;
              break;
            }

            parseOk = 1;
            break;

          case CLEAR_RX_BUFFER:
            if (ioctlIdParseStatus != PARSE_END) {
              parseOk = 0;
              break;
            }

            parseOk = 1;
            break;

          case CLEAR_PERIODIC_MSGS:
            if (ioctlIdParseStatus != PARSE_END) {
              parseOk = 0;
              break;
            }

            parseOk = 1;
            break;

          case CLEAR_MSG_FILTERS:
            if (ioctlIdParseStatus != PARSE_END) {
              parseOk = 0;
              break;
            }

            parseOk = 1;
            break;

          case CLEAR_FUNCT_MSG_LOOKUP_TABLE:
            parseOk = 1; // stub as not supported
            break;

          case ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
            parseOk = 1; // stub as not supported
            break;

          case DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
            parseOk = 1; // stub as not supported
            break;

          case READ_PROG_VOLTAGE:
            parseOk = 1; // stub as not supported
            break;

          default:
            Error_Handler();
            break;
        }
      }
      break;

    default:
      Error_Handler();
      break;
  }

  if (parseOk == 0) {
    status = IF_ERROR;
  }

  return status;
}

static uint8_t isCommand(uint8_t *cmdBuf, uint16_t cmdLen, uint8_t *cmd)
{
  uint8_t status = IF_OK;
  uint16_t i = 0;
  cmdBuf[cmdLen] = '\0'; // it should be guaranteed that this symbols is space

  uint8_t found = 1;
  while ((i < COMMAND_COUNT) && found != 0) {
    found = strcmp(commands[i++], (char*)cmdBuf);
  }

  if (found != 0) {
    status = IF_ERROR;
  }

  cmdBuf[cmdLen] = ' ';
  *cmd = i;

  return status;
}

static uint8_t parseMsg(uint8_t **iter, PassThruMessage *result)
{
  uint8_t status;

  status = (parseInt(iter, &result->ProtocolID) == PARSE_COMMA) &&
      (parseInt(iter, &result->RxStatus) == PARSE_COMMA) &&
      (parseInt(iter, &result->TxFlags) == PARSE_COMMA) &&
      (parseInt(iter, &result->Timestamp) == PARSE_COMMA) &&
      (parseInt(iter, &result->DataSize) == PARSE_COMMA) &&
      (parseInt(iter, &result->ExtraDataIndex) == PARSE_COMMA);

  if ((!status) || ((result->DataSize > 0) && (*((*iter) - 1) != ',')) ||
      ((result->DataSize == 0) && (*((*iter) - 1) != COMMAND_DELIMITER_SYMB)))
  {
    // если парсинг не удался или ожидается содержимое, но встречена не запятая,
    // или данных не ожидается, но
    status = PARSE_ERROR;
    return status;
  }

  if (result->DataSize != 0) {
    status = parseArray(iter, result->Data);
  }

  return status;
}

static uint8_t parseInt(uint8_t **iter, uint32_t *result)
{
  uint8_t status = 0;

  *result = 0;

  while ((**iter >= '0' && **iter <= '9') || (**iter >= 'a' && **iter <= 'f')) {
    *result <<= 4;

    if (**iter >= '0' && **iter <= '9') {
      *result += **iter - '0';
    } else {
      *result += **iter - 'a' + 10;
    }

    ++(*iter);
  }

  switch (**iter) {
    case ',':
      status = PARSE_COMMA;
      break;

    case COMMAND_DELIMITER_SYMB:
      status = PARSE_END;
      break;

    default:
      status = PARSE_ERROR;
      break;
  }

  ++(*iter);

  return status;
}

static uint8_t parseArray(uint8_t **iter, uint8_t *result)
{
  uint8_t status = 0;
  uint8_t error = 0;
  uint16_t i = 0;

  while (((**iter >= '0' && **iter <= '9') || (**iter >= 'a' && **iter <= 'f')) && !error) {
    uint8_t num = 0;

    if (**iter >= '0' && **iter <= '9') {
      num += **iter - '0';
    } else {
      num += **iter - 'a' + 10;
    }
    num <<= 4;

    ++(*iter);

    if (**iter >= '0' && **iter <= '9') {
      num += **iter - '0';
    } else if (**iter >= 'a' && **iter <= 'f') {
      num += **iter - 'a' + 10;
    } else {
      error = 1;
    }

    result[i] = num;
    ++(*iter);
  }

  if (!error) {
    switch (**iter) {
      case ',':
        status = PARSE_COMMA;
        break;

      case COMMAND_DELIMITER_SYMB:
        status = PARSE_END;
        break;

      default:
        status = PARSE_ERROR;
        break;
    }
  } else {
    status = PARSE_ERROR;
  }

  ++(*iter);

  return status;
}

static void prepareIntToSend(uint32_t num, uint8_t *buf)
{
  for (int i = 7; i >= 0; --i) {
    if ((num & 0x0FUL) <= 9) {
      buf[i] = '0' + (uint8_t)(num & 0x0FUL);
    } else {
      buf[i] = 'A' + (uint8_t)(num & 0x0FUL) - 10;
    }
    num >>= 4;
  }
  buf[8] = '\r';
}

static void prepareMsgToSend(PassThruMessage *msg, uint8_t *buf, uint16_t *len)
{
  int i = 0;
  for (i = 0; i < msg->DataSize; ++i) {
    uint8_t num = msg->Data[i];

    if ((num & 0x0FUL) <= 9) {
      buf[i*2] = '0' + (uint8_t)(num & 0x0FUL);
    } else {
      buf[i*2] = 'A' + (uint8_t)(num & 0x0FUL) - 10;
    }

    num >>= 4;

    if ((num & 0x0FUL) <= 9) {
      buf[i*2+1] = '0' + (uint8_t)(num & 0x0FUL);
    } else {
      buf[i*2+1] = 'A' + (uint8_t)(num & 0x0FUL) - 10;
    }
  }
  buf[i*2] = '\r';
  buf[i*2+1] = '\0';
  *len = msg->DataSize * 2;
}

static void sendErrorCode(uint32_t code)
{
  uint8_t len = strlen(error_string[code]);
  sendVcpData(error_string[code], len);
}

static void sendVcpData(uint8_t *buf, uint16_t len)
{
  while (CDC_Transmit_FS(buf, len) != USBD_OK);
}
