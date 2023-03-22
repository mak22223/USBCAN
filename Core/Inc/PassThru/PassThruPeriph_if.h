/*
 * PassThruPeriph_if.h
 *
 *  Created on: Dec 27, 2022
 *      Author: mak22
 */

#ifndef INC_PASSTHRU_PASSTHRUPERIPH_IF_H_
#define INC_PASSTHRU_PASSTHRUPERIPH_IF_H_

#include "PassThru_def.h"

typedef struct {
  PassThruError (* Init)(void *this, void *params);
  PassThruError (* Connect)(void *this, PassThruParams *params);
  PassThruError (* Disconnect)(void *this);
  PassThruError (* ReadMsgs)(void *this, PassThruParams *params);
  PassThruError (* WriteMsgs)(void *this, PassThruParams *params);
  PassThruError (* SetFilter)(void *this, PassThruParams *params);
  PassThruError (* ResetFilter)(void *this, PassThruParams *params);
  PassThruError (* HandleIoctl)(void *this, PassThruParams *params);

  void (* interruptHandler)(void *this);

  uint8_t (* isConnected)(void *this);
  uint8_t (* isCapableOf)(void *this, PassThruProtocolId protocol);
} PassThruPeriph_ItfTypeDef;

#endif /* INC_PASSTHRU_PASSTHRUPERIPH_IF_H_ */
