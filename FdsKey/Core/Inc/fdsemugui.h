#ifndef INC_FDSEMUGUI_H_
#define INC_FDSEMUGUI_H_

#include "app_fatfs.h"

#define FDS_GUI_GAME_NAME_FONT FONT_SLIMFONT_8
#define FDS_GUI_HORIZONTAL_SCROLL_SPEED 2
#define FDS_GUI_HORIZONTAL_SCROLL_PAUSE 24
#define FDS_GUI_FILE_NUMBER_FONT FONT_DIGITS
#define FDS_GUI_SIDE_SWITCH_DELAY 800

FRESULT fds_gui_load_side(char *filename, char *game_name, uint8_t *side, uint8_t side_count, uint8_t ro);

#endif /* INC_FDSEMUGUI_H_ */
