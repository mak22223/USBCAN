/*
 * Button.cpp
 *
 *  Created on: Dec 22, 2022
 *      Author: mak22
 */

#include "Button.h"

void Btn_Init(Button_TypeDef *btn, GPIO_TypeDef* _GPIO, uint16_t _GPIO_Pin, GPIO_PinState _triggLevel)
{
  btn->triggLevel = _triggLevel;
  btn->GPIO = _GPIO;
  btn->GPIO_Pin = _GPIO_Pin;

  btn->dbTim = HAL_GetTick();
  btn->pressed = !_triggLevel;
}

uint8_t Btn_Check(Button_TypeDef *btn)
{
  if (btn->dbTim + BTN_DEBOUNCE_DLY < HAL_GetTick()) {
    GPIO_PinState state = HAL_GPIO_ReadPin(btn->GPIO, btn->GPIO_Pin);

    if (state != btn->pressed) {
      btn->dbTim = HAL_GetTick();
      btn->pressed = state;

      return btn->pressed == btn->triggLevel;
    }
  }
  return 0;
}
