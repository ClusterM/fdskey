#ifndef INC_SPLASH_H_
#define INC_SPLASH_H_

#include "fatfs.h"

#define SPLASH_LOADING_IMAGE IMAGE_MICROSD

void show_loading_screen();
void show_saving_screen();
void show_error_screen(char *text);
void show_error_screen_fr(FRESULT fr);

#endif /* INC_SPLASH_H_ */
