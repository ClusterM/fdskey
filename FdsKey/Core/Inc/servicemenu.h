#ifndef INC_SERVICEMENU_H_
#define INC_SERVICEMENU_H_

#include <stdint.h>
#include "main.h"
#include "oled.h"

#define SERVICE_SETTINGS_SIGNATURE "SFDSKEY"

#define SERVICE_SETTINGS_FLASH_OFFSET (0x08080000 - FLASH_PAGE_SIZE * 3)
#define SERVICE_SETTINGS_ITEM_COUNT 4

typedef enum
{
  SERVICE_SETTING_OLED_CONTROLLER = 0,
  SERVICE_SETTING_COMMIT,
  SERVICE_SETTING_BUILD_DATE,
  SERVICE_SETTING_BUILD_TIME
} SERVICE_SETTING_ID;

typedef struct __attribute__((packed))
{
  char sig[sizeof(SERVICE_SETTINGS_SIGNATURE) + 1];
  OLED_CONTROLLER oled_controller;
} FDSKEY_SERVICE_SETTINGS;

extern FDSKEY_SERVICE_SETTINGS fdskey_service_settings;

void service_settings_load();
HAL_StatusTypeDef service_settings_save();
void service_menu();

#endif /* INC_SERVICEMENU_H_ */
