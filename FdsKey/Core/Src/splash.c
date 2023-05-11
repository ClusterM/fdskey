#include <stdio.h>
#include <string.h>
#include "splash.h"
#include "oled.h"
#include "app_fatfs.h"
#include "fdsemu.h"
#include "buttons.h"

void show_message(char *text, uint8_t wait)
{
  char* second_line;

  oled_draw_rectangle(0, oled_get_line() + OLED_HEIGHT, OLED_WIDTH - 1, oled_get_line() + OLED_HEIGHT * 2 - 1, 1, 0);

  second_line = strstr(text, "\n");
  if (!second_line)
  {
    oled_draw_text(&SPLASH_REGULAR_FONT, text, OLED_WIDTH / 2 - oled_get_text_length(&SPLASH_REGULAR_FONT, text) / 2,
      oled_get_line() + OLED_HEIGHT + OLED_HEIGHT / 2 - SPLASH_REGULAR_FONT.char_height / 2, 0, 0);
  } else {
    // two-lined text
    second_line++;
    // we need to make a copy of text because text can point to non-writable memory
    char first_line[second_line - text];
    memcpy(first_line, text, second_line - text - 1);
    first_line[second_line - text - 1] = 0;
    oled_draw_text(&SPLASH_REGULAR_FONT, first_line, OLED_WIDTH / 2 - oled_get_text_length(&SPLASH_REGULAR_FONT, first_line) / 2,
      oled_get_line() + OLED_HEIGHT + OLED_HEIGHT / 2 - SPLASH_REGULAR_FONT.char_height, 0, 0);
    oled_draw_text(&SPLASH_REGULAR_FONT, second_line, OLED_WIDTH / 2 - oled_get_text_length(&SPLASH_REGULAR_FONT, second_line) / 2,
      oled_get_line() + OLED_HEIGHT + OLED_HEIGHT / 2, 0, 0);
  }

  oled_update_invisible();
  oled_switch_to_invisible();

  if (wait)
    while (!button_up_newpress() && !button_down_newpress() && !button_left_newpress() && !button_right_newpress());
}

void show_loading_screen()
{
  show_message("Loading", 0);
}

void show_saving_screen()
{
  oled_draw_rectangle(0, oled_get_line() + OLED_HEIGHT, OLED_WIDTH - 1, oled_get_line() + OLED_HEIGHT * 2 - 1, 1, 0);
  oled_draw_image(&SPLASH_LOADING_IMAGE, OLED_WIDTH - SPLASH_LOADING_IMAGE.width - 10, oled_get_line() + OLED_HEIGHT + (OLED_HEIGHT - SPLASH_LOADING_IMAGE.height) / 2, 0, 0);
  oled_draw_text(&SPLASH_REGULAR_FONT, "Saving", 10, oled_get_line() + OLED_HEIGHT + 11, 0, 0);
  oled_update_invisible();
  oled_switch_to_invisible();
}

void show_error_screen(char *text, uint8_t fatal)
{
  if (fatal)
    __disable_irq();

  oled_draw_rectangle(0, oled_get_line() + OLED_HEIGHT, OLED_WIDTH - 1, oled_get_line() + OLED_HEIGHT * 2 - 1, 1, 0);
  oled_draw_text(&SPLASH_ERROR_TITLE_FONT, "Error :(", 4, oled_get_line() + OLED_HEIGHT, 0, 0);
  oled_draw_text(&SPLASH_REGULAR_FONT, text, 4, oled_get_line() + OLED_HEIGHT + 20, 0, 0);
  oled_update_invisible();
  oled_switch_to_invisible();
  oled_send_command(OLED_CMD_SET_ON);
  HAL_GPIO_WritePin(FDS_MEDIA_SET_GPIO_Port, FDS_MEDIA_SET_Pin, GPIO_PIN_SET);

  if (fatal)
  {
    while (1);
  } else {
    while (!button_up_newpress() && !button_down_newpress() && !button_left_newpress() && !button_right_newpress());
  }
}

void show_error_screen_fr(FRESULT fr, uint8_t fatal)
{
  char *text;
  switch ((int)fr)
  {
    case FR_OK: return;        /* (0) Succeeded */
    case FR_DISK_ERR: text = "SD card error"; break;      /* (1) A hard error occurred in the low level disk I/O layer */
    case FR_INT_ERR: text = "Assertion failed"; break;       /* (2) Assertion failed */
    case FR_NOT_READY: text = "SD card not ready"; break;     /* (3) The physical drive cannot work */
    case FR_NO_FILE: text = "File not found"; break;       /* (4) Could not find the file */
    case FR_NO_PATH: text = "Path not found"; break;       /* (5) Could not find the path */
    case FR_INVALID_NAME: text = "Invalid path name"; break;    /* (6) The path name format is invalid */
    case FR_DENIED: text = "Access denied"; break;        /* (7) Access denied due to prohibited access or directory full */
    case FR_EXIST: text = "Already exists"; break;       /* (8) Access denied due to prohibited access */
    case FR_INVALID_OBJECT: text = "Invalid object"; break;    /* (9) The file/directory object is invalid */
    case FR_WRITE_PROTECTED: text = "Write protected"; break;   /* (10) The physical drive is write protected */
    case FR_INVALID_DRIVE: text = "Invalid drive"; break;   /* (11) The logical drive number is invalid */
    case FR_NOT_ENABLED: text = "Not enabled"; break;     /* (12) The volume has no work area */
    case FR_NO_FILESYSTEM: text = "No filesystem"; break;   /* (13) There is no valid FAT volume */
    case FR_MKFS_ABORTED: text = "MKFS aborted"; break;    /* (14) The f_mkfs() aborted due to any problem */
    case FR_TIMEOUT: text = "SD card timeout"; break;       /* (15) Could not get a grant to access the volume within defined period */
    case FR_LOCKED: text = "Locked"; break;        /* (16) The operation is rejected according to the file sharing policy */
    case FR_NOT_ENOUGH_CORE: text = "Not enough core"; break;   /* (17) LFN working buffer could not be allocated */
    case FR_TOO_MANY_OPEN_FILES: text = "Too many open files"; break; /* (18) Number of open files > _FS_LOCK */
    case FR_INVALID_PARAMETER: text = "Invalid parameter"; break;  /* (19) Given parameter is invalid */
    case FDSR_WRONG_CRC: text = "Wrong block CRC"; break;
    case FDSR_INVALID_ROM: text = "Invalid ROM"; break;
    case FDSR_OUT_OF_MEMORY: text = "Out of memory"; break;
    case FDSR_ROM_TOO_LARGE: text = "ROM is too large"; break;
    case FDSR_READ_ONLY: text = "ROM is write-protected"; break;
    default: text = "Unknown error";
  }
  show_error_screen(text, fatal);
}

void show_free_memory()
{
  // for debugging only
  int i;
  void ** pointers;
  i = 0;
  int mem;
  char text[16];
  pointers = malloc(1024 * sizeof(void*));
  while ((pointers[i] = malloc(1024)))
  {
    i++;
  }
  mem = i;
  i--;
  for (; i >= 0; i--)
  {
    free(pointers[i]);
  }
  free(pointers);

  oled_draw_rectangle(0, oled_get_line() + OLED_HEIGHT, OLED_WIDTH - 1, oled_get_line() + OLED_HEIGHT * 2 - 1, 1, 0);
  oled_draw_text(&FONT_GAMEGIRL_CLASSIC_6, "FREE MEMORY", 3, oled_get_line() + OLED_HEIGHT + 3, 0, 0);
  sprintf(text, "%d KiB", mem);
  oled_draw_text(&FONT_GAMEGIRL_CLASSIC_6, text, 3, oled_get_line() + OLED_HEIGHT + 20, 0, 0);
  oled_update_invisible();
  oled_switch_to_invisible();

  while (!button_up_newpress() && !button_down_newpress() && !button_left_newpress() && !button_right_newpress());
}
