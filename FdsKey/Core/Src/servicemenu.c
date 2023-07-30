#include <string.h>
#include <stdio.h>
#include "servicemenu.h"
#include "main.h"
#include "settings.h"
#include "oled.h"
#include "buttons.h"
#include "commit.h"
#include "splash.h"
#include "confirm.h"
#include "sdcard.h"
#include "blupdater.h"

FDSKEY_SERVICE_SETTINGS fdskey_service_settings;
FDSKEY_HARDWARE_VERSION fdskey_hw_version;
static uint64_t fat_free;
static uint64_t fat_total;

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

static void uint64_to_str(uint64_t d, char* str)
{
  int len = 0, p;
  uint64_t tmp;
  for (tmp = d; tmp; tmp /= 10)
    len++;
  if (!len) len++;
  len += (len - 1) / 3;
  str[len] = 0;
  for (p = 1; len; d /= 10, len--, p++)
  {
    if (p % 4 == 0)
    {
      str[len - 1] = '\'';
      len--;
      p++;
    }
    str[len - 1] = '0' + (d % 10);
  }
}

static void draw_item(uint8_t line, SETTING_ID item, uint8_t is_selected)
{
  char *parameter_name;
  char value_v[32] = "";
  char *value = value_v;
  int l;
  SD_CID cid;

  switch((int)item)
  {
  case SERVICE_SETTING_SD_MANUFACTURER_ID:
  case SERVICE_SETTING_SD_OEM_ID:
  case SERVICE_SETTING_SD_PROD_NAME:
  case SERVICE_SETTING_SD_PROD_REV:
  case SERVICE_SETTING_SD_PROD_SN:
  case SERVICE_SETTING_SD_PROD_MANUFACT_YEAR:
  case SERVICE_SETTING_SD_PROD_MANUFACT_MONTH:
    memset(&cid, 0, sizeof(cid));
    SD_read_CID(&cid);
    break;
  default:
    break;
  }

  switch ((int)item)
  {
  case SERVICE_SETTING_OLED_CONTROLLER:
    parameter_name = "OLED controller";
    sprintf(value_v, "<%s>", fdskey_service_settings.oled_controller == OLED_CONTROLLER_SSD1306 ? "SSD1306" : "SH1106");
    break;
  case SERVICE_SETTING_VERSION:
    parameter_name = "FW version";
#ifndef INTERIM
    if (!FDSKEY_VERION_SUFFIX)
      sprintf(value_v, "v%d.%d", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR);
    else
      sprintf(value_v, "v%d.%d%c", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR, FDSKEY_VERION_SUFFIX);
#else
    value = "interim";
#endif
    break;
  case SERVICE_SETTING_COMMIT:
    parameter_name = "FW commit";
    value = COMMIT;
    break;
  case SERVICE_SETTING_BUILD_DATE:
    parameter_name = "FW build date";
    value = BUILD_DATE;
    break;
  case SERVICE_SETTING_BUILD_TIME:
    parameter_name = "FW build time";
    value = BUILD_TIME;
    break;
  case SERVICE_SETTING_BL_COMMIT:
    parameter_name = "BL commit";
    value = fdskey_hw_version.bootloader_commit;
    break;
  case SERVICE_SETTING_SD_CAPACITY:
    parameter_name = "SD size";
    uint64_to_str(SD_read_capacity(), value_v);
    break;
  case SERVICE_SETTING_FAT_SIZE:
    parameter_name = "FAT size";
    uint64_to_str(fat_total, value_v);
    break;
  case SERVICE_SETTING_FAT_FREE:
    parameter_name = "FAT free";
    uint64_to_str(fat_free, value_v);
    break;
  case SERVICE_SETTING_FILE_SYSTEM:
    parameter_name = "File system";
    switch (USERFatFs.fs_type)
    {
    case FS_FAT12:
      value = "FAT12";
      break;
    case FS_FAT16:
      value = "FAT16";
      break;
    case FS_FAT32:
      value = "FAT32";
      break;
    case FS_EXFAT:
      value = "exFAT";
      break;
    default:
      value = "Unknown";
      break;
    }
    break;  case SERVICE_SETTING_SD_SPI_SPEED:
    parameter_name = "SD SPI speed";
    switch (SD_get_spi_speed())
    {
    case SPI_BAUDRATEPRESCALER_2:
      value = "/2";
      break;
    case SPI_BAUDRATEPRESCALER_4:
      value = "/4";
      break;
    case SPI_BAUDRATEPRESCALER_8:
      value = "/8";
      break;
    case SPI_BAUDRATEPRESCALER_16:
      value = "/16";
      break;
    default:
      value = "unknown";
      break;
    }
    break;
  case SERVICE_SETTING_SD_MANUFACTURER_ID:
    parameter_name = "SD manufacturer ID";
    sprintf(value_v, "%02X", cid.ManufacturerID);
    break;
  case SERVICE_SETTING_SD_OEM_ID:
    parameter_name = "SD OEM ID";
    value = cid.OEM_AppliID;
    break;
  case SERVICE_SETTING_SD_PROD_NAME:
    parameter_name = "SD product name";
    value = cid.ProdName;
    break;
  case SERVICE_SETTING_SD_PROD_REV:
    parameter_name = "SD product rev.";
    sprintf(value_v, "%d", cid.ProdRev);
    break;
  case SERVICE_SETTING_SD_PROD_SN:
    parameter_name = "SD product s/n";
    sprintf(value_v, "%08X", (unsigned int)cid.ProdSN);
    break;
  case SERVICE_SETTING_SD_PROD_MANUFACT_YEAR:
    parameter_name = "SD manuf. year";
    sprintf(value_v, "%d", cid.ManufactYear);
    break;
  case SERVICE_SETTING_SD_PROD_MANUFACT_MONTH:
    parameter_name = "SD manuf. month";
    sprintf(value_v, "%d", cid.ManufactMonth);
    break;
  case SERVICE_SETTING_SD_FORMAT:
    parameter_name = "[ Format SD card ]";
    break;
  case SERVICE_SETTING_BL_UPDATE:
    parameter_name = "[ Update bootloader ]";
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

static void draw_all(int line, int selection)
{
  int i;
  for (i = 0; i < 4; i++)
    draw_item((oled_get_line() + OLED_HEIGHT) / 8 + i, line + i, line + i == selection);
  oled_switch_to_invisible();
}

void service_menu()
{
  int line = 0;
  int selection = 0;
  int i;
  FATFS *fs;
  FRESULT fr;
  DWORD fat_free_clust = 0;

  show_message("Entering into\nthe service menu", 0);
  HAL_Delay(1500);

  fr = f_getfree("", &fat_free_clust, &fs);
  if (fr != FR_OK)
  {
    fat_total = fat_free = 0;
  } else {
    fat_total = (uint64_t)(fs->n_fatent - 2) * fs->csize * _MIN_SS;
    fat_free = (uint64_t)fat_free_clust * fs->csize * _MIN_SS;
  }

  draw_all(line, selection);

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
        draw_all(line, selection);
        break;
      case SERVICE_SETTING_VERSION:
      case SERVICE_SETTING_COMMIT:
      case SERVICE_SETTING_BUILD_DATE:
      case SERVICE_SETTING_BUILD_TIME:
      case SERVICE_SETTING_BL_COMMIT:
      case SERVICE_SETTING_SD_SPI_SPEED:
      case SERVICE_SETTING_SD_CAPACITY:
      case SERVICE_SETTING_FAT_SIZE:
      case SERVICE_SETTING_FAT_FREE:
      case SERVICE_SETTING_FILE_SYSTEM:
      case SERVICE_SETTING_SD_MANUFACTURER_ID:
      case SERVICE_SETTING_SD_OEM_ID:
      case SERVICE_SETTING_SD_PROD_NAME:
      case SERVICE_SETTING_SD_PROD_REV:
      case SERVICE_SETTING_SD_PROD_SN:
      case SERVICE_SETTING_SD_PROD_MANUFACT_YEAR:
      case SERVICE_SETTING_SD_PROD_MANUFACT_MONTH:
        break;
      case SERVICE_SETTING_SD_FORMAT:
        sd_format();
        draw_all(line, selection);
        break;
      case SERVICE_SETTING_BL_UPDATE:
        update_bootloader();
        draw_all(line, selection);
        break;
      default:
        service_settings_save();
        return;
      }
      draw_item(oled_get_line() / 8 + selection - line, selection, 1);
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
    button_check_screen_off();
    HAL_Delay(1);
  }
}

void sd_format()
{
  FRESULT fr;
  uint8_t work[32 * 1024];

  // Confirm
  if (!confirm("Format SD card?"))
    return;
  show_message("WARNING!\nAll data will be erased!", 1);
  if (!confirm("Are you sure?"))
      return;

  show_message("Formatting...", 0);
  // unmount
  f_mount(0, "", 1);
  fr = f_mkfs("", FM_ANY | FM_SFD, 0, work, sizeof(work));
  if (fr != FR_OK)
  {
    show_error_screen_fr(fr, 0);
    fr = f_mount(&USERFatFs, "", 1);
    show_error_screen_fr(fr, 1);
    return;
  }
  // get new file system
  fr = f_mount(&USERFatFs, "", 1);
  if (fr != FR_OK)
    show_error_screen_fr(fr, 1);
  // show new file system
  switch (USERFatFs.fs_type)
  {
  case FS_FAT12:
    show_message("Done!\nFormatted to FAT12", 1);
    break;
  case FS_FAT16:
    show_message("Done!\nFormatted to FAT16", 1);
    break;
  case FS_FAT32:
    show_message("Done!\nFormatted to FAT32", 1);
    break;
  case FS_EXFAT:
    show_message("Done!\nFormatted to exFAT", 1);
    break;
  default:
    show_message("Done!", 1);
    break;
  }
  f_setlabel(DISK_LABEL);
}

