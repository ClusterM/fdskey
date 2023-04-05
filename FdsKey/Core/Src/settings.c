#include <string.h>
#include <stdio.h>
#include "settings.h"
#include "main.h"
#include "oled.h"
#include "buttons.h"

FDSKEY_SETTINGS fdskey_settings;

void settings_load()
{
  memcpy(&fdskey_settings, (void*)SETTINGS_FLASH_OFFSET, sizeof(fdskey_settings));
  fdskey_settings.sig[sizeof(fdskey_settings.sig) - 1] = 0;
  if (strcmp(fdskey_settings.sig, SETTINGS_SIGNATURE))
  {
    // config is empty, load defaults
    memset(&fdskey_settings, 0, sizeof(fdskey_settings));
    strcpy(fdskey_settings.sig, SETTINGS_SIGNATURE);
    fdskey_settings.version = SETTINGS_VERSION;
    fdskey_settings.remember_last_file = 1;
    fdskey_settings.fast_rewind = 0;
    fdskey_settings.hide_non_fds = 1;
    fdskey_settings.hide_extensions = 1;
    fdskey_settings.hide_hidden = 1;
    fdskey_settings.autosave_time = 1;
    fdskey_settings.brightness = 5;
    fdskey_settings.lefty_mode = 0;
    fdskey_settings.invert_screen = 0;
    fdskey_settings.auto_off_screen_time = 60;
    fdskey_settings.backup_original = 1;
    fdskey_settings.last_directory[0] = 0;
    fdskey_settings.last_file[0] = 0;
    fdskey_settings.last_state_menu = 1;
  }
}

HAL_StatusTypeDef settings_save()
{
  HAL_StatusTypeDef r;
  FLASH_EraseInitTypeDef erase_init_struct;
  uint32_t sector_error = 0;
  int i;
  uint64_t buffer[sizeof(fdskey_settings) / sizeof(uint64_t) + 1];

  // unlock flash
  r = HAL_FLASH_Unlock();
  if (r != HAL_OK) return r;

  // erase flash page
  erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init_struct.Banks = ((SETTINGS_FLASH_OFFSET - 0x08000000) / FLASH_BANK_SIZE == 0) ? FLASH_BANK_1 : FLASH_BANK_2;
  erase_init_struct.Page = ((SETTINGS_FLASH_OFFSET - 0x08000000) / FLASH_PAGE_SIZE) % FLASH_PAGE_NB;
  erase_init_struct.NbPages = 1;
  r = HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
  if (r != HAL_OK) return r;

  // writing
  memcpy(buffer, &fdskey_settings, sizeof(fdskey_settings));
  for (i = 0; i < sizeof(buffer); i += sizeof(uint64_t))
  {
    r = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, SETTINGS_FLASH_OFFSET + i, buffer[i / sizeof(uint64_t)]);
    if (r != HAL_OK)
      return r;
  }

  return HAL_FLASH_Lock();
}

static void draw_item(uint8_t line, SETTING_ID item, uint8_t is_selected)
{
  char *parameter_name;
  char value_v[32] = "";
  char *value = value_v;
  char* off = "\x86";
  char* on = "\x87";
  int l;

  switch (item)
  {
  case SETTING_REMEMBER_LAST_FILE:
    parameter_name = "Remember last file";
    value = fdskey_settings.remember_last_file ? on : off;
    break;
  case SETTING_FAST_REWIND:
    parameter_name = "Fast disk rewind";
    value = fdskey_settings.fast_rewind ? on : off;
    break;
  case SETTING_HIDE_NON_FDS:
    parameter_name = "Hide non .fds files";
    value = fdskey_settings.hide_non_fds ? on : off;
    break;
  case SETTING_HIDE_EXTENSIONS:
    parameter_name = "Hide .fds extensions";
    value = fdskey_settings.hide_extensions ? on : off;
    break;
  case SETTING_HIDE_HIDDEN:
    parameter_name = "Hide hidden files";
    value = fdskey_settings.hide_hidden ? on : off;
    break;
  case SETTING_AUTOSAVE_TIME:
    parameter_name = "Autosave delay";
    sprintf(value_v, "<%ds>", fdskey_settings.autosave_time);
    break;
  case SETTING_BRIGHTNESS:
    parameter_name = "Brightness";
    sprintf(value_v, "<%d>", fdskey_settings.brightness);
    break;
  case SETTING_INVERT_SCREEN:
    parameter_name = "Invert screen";
    value = fdskey_settings.invert_screen ? on : off;
    break;
  case SETTING_LEFTY_MODE:
    parameter_name = "Lefty mode";
    value = fdskey_settings.lefty_mode ? on : off;
    break;
  case SETTING_AUTO_OFF_SCREEN_TIME:
    parameter_name = "Screen off after";
    if (fdskey_settings.auto_off_screen_time)
      sprintf(value_v, "<%ds>", fdskey_settings.auto_off_screen_time);
    else
      value = "<->";
    break;
  case SETTING_BACKUP_ORIGINAL:
    parameter_name = "Backup original ROM";
    value = fdskey_settings.backup_original ? on : off;
    break;
  default:
    parameter_name = "[ Save and return ]";
    break;
  }

  oled_draw_rectangle(0, line * 8, OLED_WIDTH - 1, line * 8 + 7, 1, 0);
  if (is_selected)
    oled_draw_image(&IMAGE_CURSOR, 0, line * 8, 0, 0);
  oled_draw_text(&SETTINGS_FONT, parameter_name,
      IMAGE_CURSOR.width, line * 8,
      0, 0);
  l = oled_get_text_length(&SETTINGS_FONT, value);
  oled_draw_text(&SETTINGS_FONT, value,
      OLED_WIDTH - l - 1, line * 8,
      0, 0);
  oled_update(line, line);
}

void settings_menu()
{
  int line = 0;
  int selection = 0;
  int i;

  for (i = 0; i < 4; i++)
    draw_item((oled_get_line() + OLED_HEIGHT) / 8 + i, line + i, line + i == selection);
  oled_switch_to_invisible();

  while (1)
  {
    if (button_up_newpress() && selection > 0)
    {
      draw_item(oled_get_line() / 8 + selection - line, selection, 0);
      selection--;
      draw_item(oled_get_line() / 8 + selection - line, selection, 1);
    }
    if (button_down_newpress() && selection + 1 < SETTINGS_ITEM_COUNT + 1)
    {
      draw_item(oled_get_line() / 8 + selection - line, selection, 0);
      selection++;
      draw_item(oled_get_line() / 8 + selection - line, selection, 1);
    }

    switch (selection)
    {
    case SETTING_AUTOSAVE_TIME:
    case SETTING_BRIGHTNESS:
    case SETTING_AUTO_OFF_SCREEN_TIME:
      button_left_right_repeat_enable(1);
      break;
    default:
      button_left_right_repeat_enable(0);
      break;
    }

    if (button_left_newpress() || button_right_newpress())
    {
      switch (selection)
      {
      case SETTING_REMEMBER_LAST_FILE:
        fdskey_settings.remember_last_file = !fdskey_settings.remember_last_file;
        break;
      case SETTING_FAST_REWIND:
        fdskey_settings.fast_rewind = !fdskey_settings.fast_rewind;
        break;
      case SETTING_HIDE_NON_FDS:
        fdskey_settings.hide_non_fds = !fdskey_settings.hide_non_fds;
        break;
      case SETTING_HIDE_EXTENSIONS:
        fdskey_settings.hide_extensions = !fdskey_settings.hide_extensions;
        break;
      case SETTING_HIDE_HIDDEN:
        fdskey_settings.hide_hidden = !fdskey_settings.hide_hidden;
        break;
      case SETTING_AUTOSAVE_TIME:
        if (button_left_holding())
        {
          if (fdskey_settings.autosave_time > 0)
            fdskey_settings.autosave_time--;
        } else {
          if (fdskey_settings.autosave_time < SETTINGS_AUTOSAVE_TIME_MAX)
            fdskey_settings.autosave_time++;
        }
        break;
      case SETTING_BRIGHTNESS:
        if (button_left_holding())
        {
          if (fdskey_settings.brightness > 0)
            fdskey_settings.brightness--;
        } else {
          if (fdskey_settings.brightness < SETTINGS_BRIGHTNESS_MAX)
            fdskey_settings.brightness++;
        }
        oled_send_commands(2, OLED_CMD_SET_CONTRAST_MODE, 0xFF * fdskey_settings.brightness / SETTINGS_BRIGHTNESS_MAX);
        break;
      case SETTING_INVERT_SCREEN:
        fdskey_settings.invert_screen = !fdskey_settings.invert_screen;
        oled_send_command(fdskey_settings.invert_screen ? OLED_CMD_SET_REVERSE_ON : OLED_CMD_SET_REVERSE_OFF);
        break;
      case SETTING_LEFTY_MODE:
        fdskey_settings.lefty_mode = !fdskey_settings.lefty_mode;
        oled_rotate(fdskey_settings.lefty_mode ? 0 : 1);
        break;
      case SETTING_AUTO_OFF_SCREEN_TIME:
        if (button_left_holding())
        {
          fdskey_settings.auto_off_screen_time -= SETTINGS_AUTO_OFF_SCREEN_TIME_STEP;
          if (fdskey_settings.auto_off_screen_time < 0)
            fdskey_settings.auto_off_screen_time = 0;
          if (fdskey_settings.auto_off_screen_time > SETTINGS_AUTO_OFF_SCREEN_TIME_MAX)
            fdskey_settings.auto_off_screen_time = SETTINGS_AUTO_OFF_SCREEN_TIME_MAX;
        } else {
          fdskey_settings.auto_off_screen_time += SETTINGS_AUTO_OFF_SCREEN_TIME_STEP;
          if (fdskey_settings.auto_off_screen_time < 0)
            fdskey_settings.auto_off_screen_time = 0;
          if (fdskey_settings.auto_off_screen_time > SETTINGS_AUTO_OFF_SCREEN_TIME_MAX)
            fdskey_settings.auto_off_screen_time = SETTINGS_AUTO_OFF_SCREEN_TIME_MAX;
        }
        break;
      case SETTING_BACKUP_ORIGINAL:
        fdskey_settings.backup_original = !fdskey_settings.backup_original;
        break;
      default:
        settings_save();
        return;
      }
      if (selection == SETTING_LEFTY_MODE)
        while(button_left_holding() || button_right_holding())
          HAL_Delay(1);
    }
    while (selection < line && line)
    {
      line--;
      for (i = 0; i < 8; i++) {
        oled_set_line(oled_get_line() - 1);
        HAL_Delay(5);
      }
    }
    while (selection > line + 3)
    {
      line++;
      for (i = 0; i < 8; i++) {
        oled_set_line(oled_get_line() + 1);
        HAL_Delay(5);
      }
    }
    draw_item(oled_get_line() / 8 + selection - line, selection, 1);
    button_check_screen_off();
    HAL_Delay(1);
  }
}
