#include <string.h>
#include <stdio.h>
#include "servicemenu.h"
#include "main.h"
#include "oled.h"

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
