#ifndef INC_BROWSER_H_
#define INC_BROWSER_H_

#include "app_fatfs.h"

#define BROWSER_FONT FONT_SLIMFONT_8
#define BROWSER_HORIZONTAL_SCROLL_SPEED 6
#define BROWSER_HORIZONTAL_SCROLL_PAUSE 12
#define BROWSER_MAX_PATH_LENGTH 4096
#define BROWSER_FOLDER_IMAGE IMAGE_FOLDER6
#define BROWSER_LONGPRESS_TIME 500

#define BROWSER_USE_RUSSIAN

typedef enum {
  BROWSER_BACK,
  BROWSER_DIRECTORY,
  BROWSER_FILE,
  BROWSER_FILE_LONGPRESS
} BROWSER_RESULT;

// struct like FILINFO but with dynamic filename
typedef struct {
  char* filename;
  FSIZE_t fsize;
  uint8_t fattrib;
} DYN_FILINFO;

FRESULT browser_tree(char *directory, int dir_max_len, FILINFO *fno, BROWSER_RESULT *br);

#endif /* INC_BROWSER_H_ */
