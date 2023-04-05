#ifndef INC_SERVICEMENU_H_
#define INC_SERVICEMENU_H_

#include <stdint.h>
#include "main.h"
#include "oled.h"

#define SERVICE_SETTINGS_SIGNATURE "SFDSKEY"

#define SERVICE_SETTINGS_FLASH_OFFSET (0x08080000 - FLASH_PAGE_SIZE * 3)
#define HARDWARE_VERSION_FLASH_OFFSET (0x08080000 - FLASH_PAGE_SIZE * 4)

typedef struct __attribute__((packed))
{
  char sig[sizeof(SERVICE_SETTINGS_SIGNATURE) + 1];
  OLED_CONTROLLER oled_controller;
} FDSKEY_SERVICE_SETTINGS;

extern FDSKEY_SERVICE_SETTINGS fdskey_service_settings;

typedef struct __attribute__((packed))
{
  uint16_t major;
  uint8_t minor;
  char suffix;
  char bootloader_commit[28];
} FDSKEY_HARDWARE_VERSION;

void service_settings_load();
HAL_StatusTypeDef write_hardware_version();

#endif /* INC_SERVICEMENU_H_ */
