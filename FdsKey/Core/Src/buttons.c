#include "main.h"
#include "buttons.h"
#include "settings.h"
#include "oled.h"
#include "fdsemu.h"

static uint8_t up_pressed = 0;
static uint8_t down_pressed = 0;
static uint8_t left_pressed = 0;
static uint8_t right_pressed = 0;
static uint32_t up_hold_time = 0;
static uint32_t down_hold_time = 0;
static uint32_t left_hold_time = 0;
static uint32_t right_hold_time = 0;
static uint32_t last_active_time = 0;
static uint8_t screen_on = 1;

static void button_screen_on();

uint8_t button_up_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_UP_GPIO_Port : BUTTON_DOWN_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_UP_Pin : BUTTON_DOWN_Pin);
  if (v) {
    last_active_time = HAL_GetTick();
    button_screen_on();
  }
  return v;
}

uint8_t button_down_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_DOWN_GPIO_Port : BUTTON_UP_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_DOWN_Pin : BUTTON_UP_Pin);
  if (v) {
    last_active_time = HAL_GetTick();
    button_screen_on();
  }
  return v;
}

uint8_t button_left_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_LEFT_GPIO_Port : BUTTON_RIGHT_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_LEFT_Pin : BUTTON_RIGHT_Pin);
  if (v) {
    last_active_time = HAL_GetTick();
    button_screen_on();
  }
  return v;
}

uint8_t button_right_holding()
{
  uint8_t v = !HAL_GPIO_ReadPin(!fdskey_settings.lefty_mode ? BUTTON_RIGHT_GPIO_Port : BUTTON_LEFT_GPIO_Port, !fdskey_settings.lefty_mode ? BUTTON_RIGHT_Pin : BUTTON_LEFT_Pin);
  if (v) {
    last_active_time = HAL_GetTick();
    button_screen_on();
  }
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

void button_check_screen_off()
{
  if (!screen_on && fds_get_state() != FDS_OFF && fds_get_state() != FDS_IDLE)
  {
    last_active_time = HAL_GetTick();
    button_screen_on();
    return;
  }

  if (!fdskey_settings.auto_off_screen_time || !screen_on)
    return;

  if (last_active_time + fdskey_settings.auto_off_screen_time * 1000 < HAL_GetTick())
  {
    screen_on = 0;
    oled_send_command(OLED_CMD_SET_OFF);
  }
}

static void button_screen_on()
{
  if (!screen_on)
  {
    oled_send_command(OLED_CMD_SET_ON);
    // no newpresses!
    up_pressed = 1;
    down_pressed = 1;
    left_pressed = 1;
    right_pressed = 1;
    screen_on = 1;
  }
}
