#include <string.h>
#include <stdio.h>
#include "servicemenu.h"
#include "main.h"
#include "oled.h"
#include "commit.h"

FDSKEY_SERVICE_SETTINGS fdskey_service_settings;

void service_settings_load()
{
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

HAL_StatusTypeDef write_hardware_version()
{
  FDSKEY_HARDWARE_VERSION hw;
  HAL_StatusTypeDef r;
  FLASH_EraseInitTypeDef erase_init_struct;
  uint32_t sector_error = 0;
  int i;
  uint64_t buffer[sizeof(hw) / sizeof(uint64_t) + 1];

  memcpy(&hw, (void*)HARDWARE_VERSION_FLASH_OFFSET, sizeof(hw));
  if (hw.major == 0
      && hw.minor == 0
      && hw.suffix == 0
      && strcmp(hw.bootloader_commit, COMMIT) == 0)
    return HAL_OK;

  hw.major = 0;
  hw.minor = 0;
  hw.suffix = 0;
  strcpy(hw.bootloader_commit, COMMIT);

  // unlock flash
  r = HAL_FLASH_Unlock();
  if (r != HAL_OK) return r;

  // erase flash page
  erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init_struct.Banks = ((HARDWARE_VERSION_FLASH_OFFSET - 0x08000000) / FLASH_BANK_SIZE == 0) ? FLASH_BANK_1 : FLASH_BANK_2;
  erase_init_struct.Page = ((HARDWARE_VERSION_FLASH_OFFSET - 0x08000000) / FLASH_PAGE_SIZE) % FLASH_PAGE_NB;
  erase_init_struct.NbPages = 1;
  r = HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
  if (r != HAL_OK) return r;

  // writing
  memcpy(buffer, &hw, sizeof(hw));
  for (i = 0; i < sizeof(buffer); i += sizeof(uint64_t))
  {
    r = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, HARDWARE_VERSION_FLASH_OFFSET + i, buffer[i / sizeof(uint64_t)]);
    if (r != HAL_OK)
      return r;
  }

  return HAL_FLASH_Lock();
}
