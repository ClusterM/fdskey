#ifndef INC_BROWSER_H_
#define INC_BROWSER_H_

#include "fatfs.h"

#define BROWSER_FONT FONT_SLIMFONT_8
#define BROWSER_HORIZONTAL_SCROLL_SPEED 4
#define BROWSER_HORIZONTAL_SCROLL_PAUSE 12
#define BROWSER_MAX_PATH_LENGTH 4096
#define BROWSER_FOLDER_IMAGE IMAGE_FOLDER6

typedef enum {
  BROWSER_BACK,
  BROWSER_DIRECTORY,
  BROWSER_FILE
} BROWSER_RESULT;

FRESULT browser_tree(char *directory, int dir_max_len, char *filename, int filename_max_len, BROWSER_RESULT *br);

#endif /* INC_BROWSER_H_ */
