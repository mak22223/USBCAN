/*
 * Control.h
 *
 *  Created on: 23 дек. 2022 г.
 *      Author: mak22
 */

#ifndef INC_PASSTHRU_PASSTHRUCORE_H_
#define INC_PASSTHRU_PASSTHRUCORE_H_

/* ------------------ PassThruCore includes ------------------- */

#include "PassThru/PassThru_def.h"
#include "PassThru/PassThruComm_if.h"

#include "stm32f1xx_hal.h"


/* ------------- PassThruCore struct declaration ------------------- */



/* ------------- PassThruCore function declaration ------------------- */


void PassThru_init(SPI_HandleTypeDef* _hspi);
void PassThru_tick(void);

#endif /* INC_PASSTHRU_PASSTHRUCORE_H_ */
