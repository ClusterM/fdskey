#include "main.h"
#include "buttons.h"

uint8_t up_pressed = 0;
uint8_t down_pressed = 0;
uint8_t left_pressed = 0;
uint8_t right_pressed = 0;
int up_hold_time = 0;
int down_hold_time = 0;

uint8_t button_up()
{
  uint8_t v = !HAL_GPIO_ReadPin(BUTTON_UP_GPIO_Port, BUTTON_UP_Pin);
  uint8_t newpress = v && !up_pressed;
  up_pressed = v;
  if (newpress)
    up_hold_time = HAL_GetTick();
  else if (!v)
    up_hold_time = 0;
  return newpress || (up_hold_time && (up_hold_time + BUTTONS_REPEAT_TIME < HAL_GetTick()));
}

uint8_t button_down()
{
  uint8_t v = !HAL_GPIO_ReadPin(BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin);;
  uint8_t newpress = v && !down_pressed;
  down_pressed = v;
  if (newpress)
    down_hold_time = HAL_GetTick();
  else if (!v)
    down_hold_time = 0;
  return newpress || (down_hold_time && (down_hold_time + BUTTONS_REPEAT_TIME < HAL_GetTick()));
}

uint8_t button_left()
{
  uint8_t v = !HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin);
  uint8_t newpress = v && !left_pressed;
  left_pressed = v;
  return newpress;
}

uint8_t button_right()
{
  uint8_t v = !HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin);
  uint8_t newpress = v && !right_pressed;
  right_pressed = v;
  return newpress;
}
