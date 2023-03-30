#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include <stdint.h>
#include "main.h"
#include "app_fatfs.h"

#define SETTINGS_FLASH_OFFSET (0x08080000 - FLASH_PAGE_SIZE * 2) // reserved two pages
#define SETTINGS_SIGNATURE "FDSKEY"
#define SETTINGS_VERSION 0
#define SETTINGS_FONT FONT_SLIMFONT_8
#define SETTINGS_BRIGHTNESS_MAX 10
#define SETTINGS_AUTO_OFF_SCREEN_TIME_MAX 300
#define SETTINGS_AUTO_OFF_SCREEN_TIME_STEP 5

typedef struct __attribute__((packed))
{
  char sig[sizeof(SETTINGS_SIGNATURE) + 1];
  uint8_t version;
  uint8_t remember_last_file;
  uint8_t fast_rewind;
  uint8_t hide_non_fds;
  uint8_t hide_extensions;
  uint8_t hide_hidden;
  uint16_t autosave_time;
  uint8_t brightness;
  uint8_t lefty_mode;
  uint8_t invert_screen;
  int16_t auto_off_screen_time;
  uint8_t backup_original;
  char last_directory[1024];
  char last_file[_MAX_LFN + 1 /*256*/];
  uint8_t last_state_menu;
} FDSKEY_SETTINGS;

void settings_load();
HAL_StatusTypeDef settings_save();
void settings_menu();

extern FDSKEY_SETTINGS fdskey_settings;

#endif /* INC_SETTINGS_H_ */
