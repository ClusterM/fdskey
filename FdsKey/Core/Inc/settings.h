#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include <stdint.h>
#include "main.h"
#include "ff.h"

#define SETTINGS_FLASH_OFFSET (0x08080000 - FLASH_PAGE_SIZE * 2) // reserved two pages
#define SETTINGS_SIGNATURE "FDSKEY"
#define SETTINGS_FONT FONT_SLIMFONT_8
#define SETTINGS_AUTOSAVE_TIME_MAX 10
#define SETTINGS_BRIGHTNESS_MAX 10
#define SETTINGS_AUTO_OFF_SCREEN_TIME_MAX 300
#define SETTINGS_AUTO_OFF_SCREEN_TIME_STEP 5

#define SETTINGS_ITEM_COUNT 10
typedef enum
{
  SETTING_FAST_REWIND = 0,
  SETTING_REMEMBER_LAST_STATE,
  //SETTING_AUTOSAVE_TIME,
  SETTING_BACKUP_ORIGINAL,
  SETTING_HIDE_NON_FDS,
  SETTING_HIDE_EXTENSIONS,
  SETTING_HIDE_HIDDEN,
  SETTING_BRIGHTNESS,
  SETTING_INVERT_SCREEN,
  SETTING_LEFTY_MODE,
  SETTING_AUTO_OFF_SCREEN_TIME
} SETTING_ID;

typedef enum __attribute__ ((__packed__))
{
  SAVES_REWRITE = 0,
  SAVES_REWRITE_BACKUP = 1,
  SAVES_EVERDRIVE
} SAVES_MODE;

typedef enum __attribute__ ((__packed__))
{
  REMEMBER_LAST_STATE_NONE = 0,
  REMEMBER_LAST_STATE_BROWSER,
  REMEMBER_LAST_STATE_ROM
} REMEMBER_LAST_STATE_MODE;

typedef enum __attribute__ ((__packed__))
{
  LAST_STATE_MAIN_MENU = 0,
  LAST_STATE_BROWSER,
  LAST_STATE_ROM
} LAST_STATE;

typedef enum __attribute__ ((__packed__))
{
  REWIND_SPEED_ORIGINAL = 0,
  REWIND_SPEED_FAST,
  REWIND_SPEED_TURBO
} REWIND_SPEED;

typedef struct __attribute__ ((packed))
{
  char sig[sizeof(SETTINGS_SIGNATURE) + 1];
  uint8_t version;
  REWIND_SPEED rewind_speed;
  REMEMBER_LAST_STATE_MODE remember_last_state_mode;
  LAST_STATE last_state;
  uint8_t hide_non_fds;
  uint8_t hide_extensions;
  uint8_t hide_hidden;
  uint16_t autosave_time;
  uint8_t brightness;
  uint8_t lefty_mode;
  uint8_t invert_screen;
  int16_t auto_off_screen_time;
  SAVES_MODE backup_original;
  char last_directory[3584];
  char last_file[FF_MAX_LFN + 1 /* 255 + 1 */];
} FDSKEY_SETTINGS;

void settings_load();
HAL_StatusTypeDef settings_save();
void settings_menu();

extern FDSKEY_SETTINGS fdskey_settings;

#endif /* INC_SETTINGS_H_ */
