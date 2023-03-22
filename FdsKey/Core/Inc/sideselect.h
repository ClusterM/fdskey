#ifndef INC_SIDESELECT_H_
#define INC_SIDESELECT_H_

#include "oled.h"
#include "app_fatfs.h"

#define SIDE_SELECT_GAME_NAME_FONT FONT_SLIMFONT_8
#define SIDE_SELECT_SIDE_NAME_FONT FONT_VERDANA_14_BOLD
#define SIDE_SELECT_HORIZONTAL_SCROLL_SPEED 1
#define SIDE_SELECT_HORIZONTAL_SCROLL_PAUSE 12

FRESULT fds_side_select(char *directory, FILINFO *fno);
DotMatrixImage* side_select_get_disk_image(uint8_t side);

#endif /* INC_SIDESELECT_H_ */
