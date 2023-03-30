#include <string.h>
#include <stdio.h>
#include "settings.h"
#include "main.h"
#include "oled.h"

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
    fdskey_settings.autosave_time = 5;
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

