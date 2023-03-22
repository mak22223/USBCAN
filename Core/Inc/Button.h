/*
 * Button.h
 *
 *  Created on: Dec 22, 2022
 *      Author: mak22
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "stm32f1xx_hal.h"

#define BTN_DEBOUNCE_DLY 70

typedef struct {
  GPIO_PinState triggLevel;

	GPIO_TypeDef *GPIO;
	uint16_t GPIO_Pin;
  uint32_t dbTim;

  GPIO_PinState pressed;
} Button_TypeDef;

void Btn_Init(
    Button_TypeDef *btn,
    GPIO_TypeDef* _GPIO,
    uint16_t _GPIO_Pin,
    GPIO_PinState _triggLevel);
uint8_t Btn_Check(Button_TypeDef *btn);


#endif /* BUTTON_H_ */
