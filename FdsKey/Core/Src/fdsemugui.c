#include <string.h>
#include <stdio.h>
#include "fdsemugui.h"
#include "fdsemu.h"
#include "oled.h"
#include "app_fatfs.h"
#include "buttons.h"
#include "sideselect.h"
#include "splash.h"

void fds_gui_draw(uint8_t side, uint8_t side_count, char *game_name, int text_scroll)
{
  DotMatrixImage *disk_image;
  DotMatrixImage *state_image;
  int max_width, text_width, total_scroll;
  int line = oled_get_line() + OLED_HEIGHT;
  FDS_STATE state = fds_get_state();
  int head_position = fds_get_head_position();
  int used_space = fds_get_used_space();
  int total_size = fds_get_max_size();
  int block = fds_get_block();
  int block_count = fds_get_block_count();
  int file_count;

  disk_image = side_select_get_disk_image(side, side_count);

  max_width = OLED_WIDTH - disk_image->width - 4;
  text_width = oled_get_text_length(&FDS_GUI_GAME_NAME_FONT, game_name) - 1 /*spacing*/;
  if (text_width > max_width)
  {
    total_scroll = FDS_GUI_HORIZONTAL_SCROLL_PAUSE + (text_width - max_width) + FDS_GUI_HORIZONTAL_SCROLL_PAUSE;
    text_scroll /= FDS_GUI_HORIZONTAL_SCROLL_SPEED;
    text_scroll %= total_scroll * 2;
    // two-directional
    if (text_scroll > total_scroll)
      text_scroll = total_scroll * 2 - text_scroll;
    if (text_scroll < FDS_GUI_HORIZONTAL_SCROLL_PAUSE)
      text_scroll = 0;
    else if (text_scroll >= FDS_GUI_HORIZONTAL_SCROLL_PAUSE + (text_width - max_width))
      text_scroll = text_width - max_width;
    else
      text_scroll = text_scroll - FDS_GUI_HORIZONTAL_SCROLL_PAUSE;
  } else {
    text_scroll = 0;
  }

  switch (state)
  {
  case FDS_READING:
    state_image = (DotMatrixImage*)&IMAGE_STATE_PLAY;
    break;
  case FDS_READ_WAIT_READY:
    state_image = (DotMatrixImage*)&IMAGE_STATE_FF;
    break;
  case FDS_READ_WAIT_READY_TIMER:
    state_image = (DotMatrixImage*)&IMAGE_STATE_REWIND;
    break;
  case FDS_WRITING:
  case FDS_WRITING_GAP:
  case FDS_WRITING_STOPPING:
    state_image = (DotMatrixImage*)&IMAGE_STATE_REC;
    break;
  default:
    state_image = (DotMatrixImage*)&IMAGE_STATE_PAUSE;
    break;
  }

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  // game name
  oled_draw_text_cropped(&FDS_GUI_GAME_NAME_FONT, game_name,
      1, line,
      text_scroll, max_width,
      0, 0,
      0, 0);
  // disk image
  oled_draw_image(
      disk_image, OLED_WIDTH - disk_image->width - 1, line + 1,
      0, 0);
  // vertical lines for progress bar
  oled_draw_line(3, line + 11, 3, line + 16, 1);
  oled_draw_line(90, line + 11, 90, line + 16, 1);
  // horizontal line
  oled_draw_line(4, line + 15, 89, line + 15, 1); // length = 86
  // used line
  int used_length = used_space * 87 / total_size;
  if (used_length > 87) used_length = 87;
  oled_draw_rectangle(3, line + 12, 3 + used_length, line + 14, 1, 1);
  int pos = head_position * 87 / total_size;
  if (pos > 87) pos = 87;
  oled_draw_image(&IMAGE_HEAD_CURSOR, 1 + pos, line + 16, 0, 0);
  // state image
  oled_draw_image(
      state_image,
      1, line + 21,
      0, 0);
  char file_str[32];
  if (block > 1)
    sprintf(file_str, "FILE %02d/", (block - 2) / 2 + 1);
  else if (block < 0)
    strcpy(file_str, "FILE --/");
  else
    strcpy(file_str, "FILE Hd/");
  oled_draw_text(&FDS_GUI_FILE_NUMBER_FONT, file_str, 32 + (((block_count - 2) / 2 / 10 == 1) ? 4 : 0), line + 22, 0, 0);
  file_count = (block_count - 2) / 2;
  if (file_count < 0) file_count = 0;
  sprintf(file_str, "%02d", file_count);
  oled_draw_text(&FDS_GUI_FILE_NUMBER_FONT, file_str, 80, line + 22, 0, 0);
  if (state_image != (DotMatrixImage*)&IMAGE_STATE_PAUSE) // lol
  {
    static int spinning = 0;
    spinning++;
    // draw disk spinning animation
    switch (spinning % 4)
    {
    case 0:
      oled_draw_line(110, line + 13, 113, line + 13, 1);
      break;
    case 1:
      oled_draw_line(114, line + 14, 114, line + 17, 1);
      break;
    case 2:
      oled_draw_line(111, line + 18, 113, line + 18, 1);
      break;
    case 3:
      oled_draw_line(109, line + 14, 109, line + 17, 1);
      break;
    }
  }
}

void fds_gui_draw_side_changing(uint8_t swap, int frame)
{
  int line = oled_get_line() + OLED_HEIGHT;
  char *first_line;
  char *second_line;
  DotMatrixImage *image;
  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  // text
  first_line = !swap ? "Flipping" : "Swapping";
  second_line = "the disk";
  oled_draw_text(&SPLASH_REGULAR_FONT, first_line, 20 + oled_get_text_length(&SPLASH_REGULAR_FONT, second_line) / 2 - oled_get_text_length(&SPLASH_REGULAR_FONT, first_line) / 2,
    oled_get_line() + OLED_HEIGHT + OLED_HEIGHT / 2 - SPLASH_REGULAR_FONT.char_height, 0, 0);
  oled_draw_text(&SPLASH_REGULAR_FONT, second_line, 20,
    oled_get_line() + OLED_HEIGHT + OLED_HEIGHT / 2, 0, 0);
  // disk image
  frame %= 20;
  if (frame >= 10)
    frame = 19 - frame;
  switch (frame)
  {
  default:
  case 0:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_0;
    break;
  case 1:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_1;
    break;
  case 2:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_2;
    break;
  case 3:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_3;
    break;
  case 4:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_4;
    break;
  case 5:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_5;
    break;
  case 6:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_6;
    break;
  case 7:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_7;
    break;
  case 8:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_8;
    break;
  case 9:
    image = (DotMatrixImage*)&IMAGE_DISK_FLIP_FRAME_9;
    break;
  }
  oled_draw_image(image, OLED_WIDTH - image->width - 20, oled_get_line() + OLED_HEIGHT + OLED_HEIGHT / 2 - image->height / 2, 0, 0);
}

FRESULT fds_gui_load_side(char *filename, char *game_name, uint8_t side, uint8_t side_count, uint8_t ro)
{
  FRESULT fr;
  int i, text_scroll = 0;
  uint8_t cmd;

  show_loading_screen();

  fr = fds_load_side(filename, side, ro);
  if (fr != FR_OK)
    return fr;

  while (1)
  {
    if (fds_get_state() == FDS_SAVE_PENDING)
    {
      show_saving_screen();
      fr = fds_save();
      if (fr != FR_OK)
        return fr;
    }

    cmd = 0;
    if (button_left_newpress())
        break;
    if (button_up_newpress() && side > 0)
      cmd = 1;
    if (button_down_newpress() && side + 1 < side_count)
      cmd = 2;
    if (cmd)
    {
      // need to change side
      if (fds_is_changed()) show_saving_screen();
      fr = fds_close(1);
      if (fr != FR_OK)
        return fr;
      fds_gui_draw_side_changing((cmd == 1 && !(side & 1)) || (cmd == 2 && (side & 1)), 0);
      oled_update_invisible();
      for (i = 0; i < OLED_HEIGHT; i++)
      {
        oled_set_line(oled_get_line() + (cmd == 1 ? -1 : 1));
        HAL_Delay(1);
      }
      for (i = 0; i < 20; i++)
      {
        HAL_Delay(FDS_GUI_SIDE_SWITCH_DELAY / 20);
        fds_gui_draw_side_changing((cmd == 1 && !(side & 1)) || (cmd == 2 && (side & 1)), i);
        oled_update_invisible();
        oled_switch_to_invisible();
      }
      if (cmd == 1)
        side--;
      else
        side++;
      fr = fds_load_side(filename, side, ro);
      if (fr != FR_OK)
        return fr;
      fds_gui_draw(side, side_count, game_name, text_scroll);
      oled_update_invisible();
      for (i = 0; i < OLED_HEIGHT; i++)
      {
        oled_set_line(oled_get_line() + (cmd == 1 ? -1 : 1));
        HAL_Delay(1);
      }
    }

    fds_gui_draw(side, side_count, game_name, text_scroll);
    oled_update_invisible();
    oled_switch_to_invisible();
//    if (!text_scroll)
//      oled_screenshot("ss_fds_emu_gui.bmp");
    button_check_screen_off();
    text_scroll++;
  }

  if (fds_is_changed()) show_saving_screen();
  fr = fds_close(1);

  return fr;
}
