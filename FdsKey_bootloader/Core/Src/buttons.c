#include "main.h"
#include "buttons.h"
#include "settings.h"

static uint8_t up_pressed = 0;
static uint8_t down_pressed = 0;
static uint8_t left_pressed = 0;
static uint8_t right_pressed = 0;
static uint32_t up_hold_time = 0;
static uint32_t down_hold_time = 0;
static uint32_t left_hold_time = 0;
static uint32_t right_hold_time = 0;

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
  if (newpress)
    up_hold_time = HAL_GetTick();
  else if (!v)
    up_hold_time = 0;
  return newpress || (up_hold_time && (up_hold_time + BUTTONS_REPEAT_TIME < HAL_GetTick()));
}

uint8_t button_down_newpress()
{
  uint8_t v = button_down_holding();
  uint8_t newpress = v && !down_pressed;
  down_pressed = v;
  if (newpress)
    down_hold_time = HAL_GetTick();
  else if (!v)
    down_hold_time = 0;
  return newpress || (down_hold_time && (down_hold_time + BUTTONS_REPEAT_TIME < HAL_GetTick()));
}

uint8_t button_left_newpress()
{
  uint8_t v = button_left_holding();
  uint8_t newpress = v && !left_pressed;
  left_pressed = v;
  if (newpress)
    left_hold_time = HAL_GetTick();
  else if (!v)
    left_hold_time = 0;
  left_pressed = v;
  return newpress /*|| (left_hold_time && (left_hold_time + BUTTONS_REPEAT_TIME < HAL_GetTick()))*/;
}

uint8_t button_right_newpress()
{
  uint8_t v = button_right_holding();
  uint8_t newpress = v && !right_pressed;
  right_pressed = v;
  if (newpress)
    right_hold_time = HAL_GetTick();
  else if (!v)
    right_hold_time = 0;
  right_pressed = v;
  return newpress /*|| (right_hold_time && (right_hold_time + BUTTONS_REPEAT_TIME < HAL_GetTick()))*/;
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

