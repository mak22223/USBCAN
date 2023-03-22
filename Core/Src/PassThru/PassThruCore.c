/*
 * Control.c
 *
 *  Created on: 23 дек. 2022 г.
 *      Author: mak22
 */

#include <PassThru/PassThruCore.h>
#include "PassThru/PassThruPeriph_if.h"
#include "usb_vcp/VCPCommParser.h"
//#include "MCP2515/MCP2515.h"

/* -------------- PassThruСore private defines ------------------ */

#define PERIPH_COUNT 2

/* -------- PassThruСore private functions declarations ----------- */

static PassThruError ConnectHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError DisconnectHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError ReadMsgsHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError WriteMsgsHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError StartPeriodicMsgsHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError StopPeriodicMsgsHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError StartMsgFilterHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError StopMsgFilterHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError SetProgrammingVoltageHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError ReadVersionHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError GetLastErrorHandler(PassThruParams *params, PassThruAnswer *ans);
static PassThruError IoctlHandler(PassThruParams *params, PassThruAnswer *ans);


/* ------------- PassThruСore private variables ------------------- */

PassThruComm_ItfTypeDef comm_itf;

struct {
  void *periph;
  PassThruPeriph_ItfTypeDef itf;
} periphs[PERIPH_COUNT];

CAN_MCP2515_TypeDef Can1;
CAN_MCP2515_TypeDef Can2;

PassThruCommand command_id;
PassThruParams param_buf;
PassThruAnswer answer_buf;
uint8_t *last_error_string;

const uint8_t error_strings[1][1] = {
    ""
};

/* ------------- PassThru functions definition ------------------- */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin) {
    case CAN1_INT_Pin:
      periphs[0].itf.interruptHandler(periphs[0].periph);
      break;

    case CAN2_INT_Pin:
      periphs[1].itf.interruptHandler(periphs[1].periph);
      break;

    default:
      break;
  }
}

/*
 * Init PassThruCore, set communication interface
 * @retval None
 */
void PassThru_init(SPI_HandleTypeDef* _hspi)
{
  VCP_getInterface(&comm_itf);

  periphs[0].periph = &Can1;
  periphs[1].periph = &Can2;
  MCP2515_getInterface(&periphs[0].itf);
  MCP2515_getInterface(&periphs[1].itf);

  CAN_MCP2515_InitTypeDef can1_init = {
    _hspi,
    CAN1_CS_GPIO_Port,
    CAN1_CS_Pin
  };

  CAN_MCP2515_InitTypeDef can2_init = {
    _hspi,
    CAN2_CS_GPIO_Port,
    CAN2_CS_Pin
  };

  periphs[0].itf.Init(&Can1, &can1_init);
  periphs[1].itf.Init(&Can2, &can2_init);

  HAL_NVIC_EnableIRQ(CAN1_INT_EXTI_IRQn);
  HAL_NVIC_EnableIRQ(CAN2_INT_EXTI_IRQn);
}

/*
 * Serve PassThru events
 * @retval None
 */
void PassThru_tick(void)
{
  PassThruError error = STATUS_NOERROR;

  if(comm_itf.ReceiveCmd(&command_id, &param_buf) == IF_OK) {
    switch (command_id) {
      case NO_COMMAND:

        break;

      case CONNECT:
        error = ConnectHandler(&param_buf, &answer_buf);
        break;

      case DISCONNECT:
        error = DisconnectHandler(&param_buf, &answer_buf);
        break;

      case READ_MSGS:
        error = ReadMsgsHandler(&param_buf, &answer_buf);
        break;

      case WRITE_MSGS:
        error = WriteMsgsHandler(&param_buf, &answer_buf);
        break;

      case START_PERIODIC_MSG:
        error = StartPeriodicMsgsHandler(&param_buf, &answer_buf);
        break;

      case STOP_PERIODIC_MSG:
        error = StopPeriodicMsgsHandler(&param_buf, &answer_buf);
        break;

      case START_MSG_FILTER:
        error = StartMsgFilterHandler(&param_buf, &answer_buf);
        break;

      case STOP_MSG_FILTER:
        error = StopMsgFilterHandler(&param_buf, &answer_buf);
        break;

      case SET_PROGRAMMING_VOLTAGE:
        error = SetProgrammingVoltageHandler(&param_buf, &answer_buf);
        break;

      case READ_VERSION:
        error = ReadVersionHandler(&param_buf, &answer_buf);
        break;

      case GET_LAST_ERROR:
        error = GetLastErrorHandler(&param_buf, &answer_buf);
        break;

      case IOCTL:
        error = IoctlHandler(&param_buf, &answer_buf);
        break;

      default:
        Error_Handler();
        break;
    }
  }

  if (command_id != NO_COMMAND) {
    comm_itf.SendAnswer(&command_id, &answer_buf);
  }
}

static PassThruError ConnectHandler(PassThruParams *params, PassThruAnswer *ans)
{
  /// TODO: придумать не жесткую схему определения интерфейса
  PassThruError status = STATUS_NOERROR;

  switch (params->Connect.protocolId) {
    case CAN:
      ans->Connect.channelId = 0;
      status = periphs[0].itf.Connect(periphs[0].periph, params);
      break;

    case MS_CAN:
      ans->Connect.channelId = 1;
      status = periphs[1].itf.Connect(periphs[1].periph, params);
      break;

    default:
      status = ERR_NOT_SUPPORTED;
      break;
  }

  ans->errorCode = status;
  return status;
}

static PassThruError DisconnectHandler(PassThruParams *params, PassThruAnswer *ans)
{
  if (params->Disconnect.channelId >= PERIPH_COUNT) {
    ans->errorCode = ERR_INVALID_CHANNEL_ID;
    return ERR_INVALID_CHANNEL_ID;
  }

  ans->errorCode = periphs[params->Disconnect.channelId].itf
      .Disconnect(periphs[params->Disconnect.channelId].periph);

  return ans->errorCode;
}

static PassThruError ReadMsgsHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError WriteMsgsHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError StartPeriodicMsgsHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError StopPeriodicMsgsHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError StartMsgFilterHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError StopMsgFilterHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError SetProgrammingVoltageHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError ReadVersionHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError GetLastErrorHandler(PassThruParams *params, PassThruAnswer *ans)
{

}

static PassThruError IoctlHandler(PassThruParams *params, PassThruAnswer *ans)
{

}
