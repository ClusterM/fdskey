#include <string.h>
#include <stdio.h>
#include "servicemenu.h"
#include "main.h"
#include "settings.h"
#include "oled.h"
#include "buttons.h"
#include "commit.h"

FDSKEY_SERVICE_SETTINGS fdskey_service_settings;
FDSKEY_HARDWARE_VERSION fdskey_hw_version;

void service_settings_load()
{
  memcpy(&fdskey_hw_version, (void*)HARDWARE_VERSION_FLASH_OFFSET, sizeof(fdskey_hw_version));
  memcpy(&fdskey_service_settings, (void*)SERVICE_SETTINGS_FLASH_OFFSET, sizeof(fdskey_service_settings));
  fdskey_service_settings.sig[sizeof(fdskey_service_settings.sig) - 1] = 0;
  if (strcmp(fdskey_service_settings.sig, SERVICE_SETTINGS_SIGNATURE))
  {
    // config is empty, load defaults
    memset(&fdskey_service_settings, 0, sizeof(fdskey_service_settings));
    strcpy(fdskey_service_settings.sig, SERVICE_SETTINGS_SIGNATURE);
    fdskey_service_settings.oled_controller = OLED_CONTROLLER_SSD1306;
  }
}

HAL_StatusTypeDef service_settings_save()
{
  HAL_StatusTypeDef r;
  FLASH_EraseInitTypeDef erase_init_struct;
  uint32_t sector_error = 0;
  int i;
  uint64_t buffer[sizeof(fdskey_service_settings) / sizeof(uint64_t) + 1];

  // unlock flash
  r = HAL_FLASH_Unlock();
  if (r != HAL_OK) return r;

  // erase flash page
  erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init_struct.Banks = ((SERVICE_SETTINGS_FLASH_OFFSET - 0x08000000) / FLASH_BANK_SIZE == 0) ? FLASH_BANK_1 : FLASH_BANK_2;
  erase_init_struct.Page = ((SERVICE_SETTINGS_FLASH_OFFSET - 0x08000000) / FLASH_PAGE_SIZE) % FLASH_PAGE_NB;
  erase_init_struct.NbPages = 1;
  r = HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
  if (r != HAL_OK) return r;

  // writing
  memcpy(buffer, &fdskey_service_settings, sizeof(fdskey_service_settings));
  for (i = 0; i < sizeof(buffer); i += sizeof(uint64_t))
  {
    r = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, SERVICE_SETTINGS_FLASH_OFFSET + i, buffer[i / sizeof(uint64_t)]);
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
//  char* off = "\x86";
//  char* on = "\x87";
  int l;

  switch (item)
  {
  case SERVICE_SETTING_OLED_CONTROLLER:
    parameter_name = "OLED controller";
    sprintf(value_v, "<%s>", fdskey_service_settings.oled_controller == OLED_CONTROLLER_SSD1306 ? "SSD1306" : "SH1106");
    break;
  case SERVICE_SETTING_VERSION:
    parameter_name = "Commit";
    if (!FDSKEY_VERION_SUFFIX)
      sprintf(value_v, "v%d.%d", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR);
    else
      sprintf(value_v, "v%d.%d%c", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR, FDSKEY_VERION_SUFFIX);
    break;
  case SERVICE_SETTING_COMMIT:
    parameter_name = "Commit";
    value = COMMIT;
    break;
  case SERVICE_SETTING_BUILD_DATE:
    parameter_name = "Build date";
    value = BUILD_DATE;
    break;
  case SERVICE_SETTING_BUILD_TIME:
    parameter_name = "Build time";
    value = BUILD_TIME;
    break;
  case SERVICE_SETTING_HW_VERSION:
    parameter_name = "HW version";
    if (!fdskey_hw_version.suffix)
      sprintf(value_v, "v%d.%d", fdskey_hw_version.major, fdskey_hw_version.minor);
    else
      sprintf(value_v, "v%d.%d%c", fdskey_hw_version.major, fdskey_hw_version.minor, fdskey_hw_version.suffix);
    break;
  case SERVICE_SETTING_BL_COMMIT:
    parameter_name = "BL commit";
    value = fdskey_hw_version.bootloader_commit;
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

void service_menu()
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
    if (button_down_newpress() && selection + 1 < SERVICE_SETTINGS_ITEM_COUNT + 1)
    {
      draw_item(oled_get_line() / 8 + selection - line, selection, 0);
      selection++;
      draw_item(oled_get_line() / 8 + selection - line, selection, 1);
    }

    if (button_left_newpress() || button_right_newpress())
    {
      switch (selection)
      {
      case SERVICE_SETTING_OLED_CONTROLLER:
        if (fdskey_service_settings.oled_controller == OLED_CONTROLLER_SSD1306)
          fdskey_service_settings.oled_controller = OLED_CONTROLLER_SH1106;
        else
          fdskey_service_settings.oled_controller = OLED_CONTROLLER_SSD1306;
        // reinit display
        oled_init(fdskey_service_settings.oled_controller, fdskey_settings.lefty_mode ? 0 : 1, fdskey_settings.invert_screen, 0xFF * fdskey_settings.brightness / SETTINGS_BRIGHTNESS_MAX);
        for (i = 0; i < 4; i++)
          draw_item((oled_get_line() + OLED_HEIGHT) / 8 + i, line + i, line + i == selection);
        oled_switch_to_invisible();
        break;
      case SERVICE_SETTING_COMMIT:
      case SERVICE_SETTING_BUILD_DATE:
      case SERVICE_SETTING_BUILD_TIME:
        break;
      default:
        service_settings_save();
        return;
      }
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
