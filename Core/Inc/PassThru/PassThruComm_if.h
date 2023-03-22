/*
 * PassThruComm_if.h
 *
 *  Created on: Dec 27, 2022
 *      Author: mak22
 */

#ifndef INC_PASSTHRU_PASSTHRUCOMM_IF_H_
#define INC_PASSTHRU_PASSTHRUCOMM_IF_H_

#include "PassThru_def.h"

typedef struct {
  uint8_t (* Init)(void);
  uint8_t (* DeInit)(void);
  uint8_t (* ReceiveCmd)(uint8_t *cmd, PassThruParams *params);
  uint8_t (* SendAnswer)(uint8_t *cmd, PassThruAnswer *ans);
} PassThruComm_ItfTypeDef;

typedef enum {
  IF_OK = 0U,
  IF_ERROR
} PassThruCommError;

#endif /* INC_PASSTHRU_PASSTHRUCOMM_IF_H_ */
