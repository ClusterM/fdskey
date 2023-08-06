#ifndef INC_SPLASH_H_
#define INC_SPLASH_H_

#include "ff.h"
#include "sdcard.h"

#define SPLASH_UPDATING_IMAGE IMAGE_COGS
#define SPLASH_REGULAR_FONT FONT_STANDARD_6
#define SPLASH_ERROR_TITLE_FONT FONT_VERDANA_12_BOLD
#define SPLASH_ERROR_FONT FONT_STANDARD_6

void show_message(char *text, uint8_t wait);
void show_updating_screen();
void show_error_screen(char *text, uint8_t fatal);
void show_error_screen_fr(FRESULT fr, uint8_t fatal);
void show_error_screen_sd(SD_RESULT r, uint8_t fatal);
void show_free_memory();

#endif /* INC_SPLASH_H_ */
