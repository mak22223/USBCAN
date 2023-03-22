/*
 * VCPCommParser.h
 *
 *  Created on: 23 дек. 2022 г.
 *      Author: mak22
 */

#ifndef COMCOMMANDPARSER_H_
#define COMCOMMANDPARSER_H_


/* ------------------ VCP command parser includes ----------------- */

#include <PassThru/PassThruComm_if.h>

#include "usb_vcp/usbd_cdc_if.h"

/* ------------------ VCP command parser defines ----------------- */

/// TODO: необходима очередь или спец обработчик.
/// Два сообщения подряд могут не отправиться из-за ожидания отправки.



/* ---------- VCP command parser public function declaration ----------- */

void VCP_getInterface(PassThruComm_ItfTypeDef *interface);

#endif /* COMCOMMANDPARSER_H_ */
