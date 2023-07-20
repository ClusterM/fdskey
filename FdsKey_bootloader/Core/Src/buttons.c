#include "main.h"
#include "buttons.h"
#include "settings.h"

static uint8_t up_pressed = 0;
static uint8_t down_pressed = 0;
static uint8_t left_pressed = 0;
static uint8_t right_pressed = 0;

uint8_t button_up_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_UP_GPIO_Port : BUTTON_DOWN_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_UP_Pin : BUTTON_DOWN_Pin);
  return v;
}

uint8_t button_down_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_DOWN_GPIO_Port : BUTTON_UP_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_DOWN_Pin : BUTTON_UP_Pin);
  return v;
}

uint8_t button_left_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_LEFT_GPIO_Port : BUTTON_RIGHT_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_LEFT_Pin : BUTTON_RIGHT_Pin);
  return v;
}

uint8_t button_right_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_RIGHT_GPIO_Port : BUTTON_LEFT_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_RIGHT_Pin : BUTTON_LEFT_Pin);
  return v;
}

uint8_t button_up_newpress()
{
  uint8_t v = button_up_holding();
  uint8_t newpress = v && !up_pressed;
  up_pressed = v;
  return newpress;
}

uint8_t button_down_newpress()
{
  uint8_t v = button_down_holding();
  uint8_t newpress = v && !down_pressed;
  down_pressed = v;
  return newpress;
}

uint8_t button_left_newpress()
{
  uint8_t v = button_left_holding();
  uint8_t newpress = v && !left_pressed;
  left_pressed = v;
  return newpress;
}

uint8_t button_right_newpress()
{
  uint8_t v = button_right_holding();
  uint8_t newpress = v && !right_pressed;
  right_pressed = v;
  return newpress;
}

uint32_t button_left_hold_time()
{
  if (!button_left_holding())
    left_hold_time = 0;
  if (!left_hold_time) return 0;
  return HAL_GetTick() - left_hold_time;
}

uint32_t button_right_hold_time()
{
  if (!button_right_holding())
    right_hold_time = 0;
  if (!right_hold_time) return 0;
  return HAL_GetTick() - right_hold_time;
}

