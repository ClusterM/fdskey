#include <fdsemugui.h>
#include <string.h>
#include "sideselect.h"
#include "fdsemu.h"
#include "oled.h"
#include "app_fatfs.h"
#include "splash.h"
#include "buttons.h"

static void fds_side_draw(uint8_t side, uint8_t side_count, char* game_name, int text_scroll)
{
  char *side_name;
  DotMatrixImage *disk_image;
  int max_width, text_width, total_scroll;
  int line = oled_get_line() + OLED_HEIGHT;
  int init_text_scroll = text_scroll;
  int l;

  switch (side)
  {
  default:
  case 0:
    if (side_count <= 2)
      side_name = "A";
    else
      side_name = "1A";
    break;
  case 1:
    if (side_count <= 2)
      side_name = "B";
    else
      side_name = "1B";
    break;
  case 2:
    side_name = "2A";
    break;
  case 3:
    side_name = "2B";
    break;
  case 4:
    side_name = "3A";
    break;
  case 5:
    side_name = "3B";
    break;
  case 6:
    side_name = "4A";
    break;
  case 7:
    side_name = "4B";
    break;
  }
  l = strlen(side_name);

  disk_image = side_select_get_disk_image(side);

  max_width = OLED_WIDTH - disk_image->width - 4;
  text_width = oled_get_text_length(&SIDE_SELECT_GAME_NAME_FONT, game_name) - 1 /*spacing*/;
  if (text_width > max_width)
  {
    total_scroll = SIDE_SELECT_HORIZONTAL_SCROLL_PAUSE + (text_width - max_width) + SIDE_SELECT_HORIZONTAL_SCROLL_PAUSE;
    text_scroll /= SIDE_SELECT_HORIZONTAL_SCROLL_SPEED;
    text_scroll %= total_scroll * 2;

    // two-directional
    if (text_scroll > total_scroll)
      text_scroll = total_scroll * 2 - text_scroll;
    if (text_scroll < SIDE_SELECT_HORIZONTAL_SCROLL_PAUSE)
      text_scroll = 0;
    else if (text_scroll >= SIDE_SELECT_HORIZONTAL_SCROLL_PAUSE + (text_width - max_width))
      text_scroll = text_width - max_width;
    else
      text_scroll = text_scroll - SIDE_SELECT_HORIZONTAL_SCROLL_PAUSE;
  } else {
    text_scroll = 0;
  }

  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  oled_draw_text_cropped(&SIDE_SELECT_GAME_NAME_FONT, game_name,
      1, line,
      text_scroll, max_width,
      0, 0,
      0, 0);
  oled_draw_image(
      disk_image, OLED_WIDTH - disk_image->width - 1, line + 1,
      0, 0);
  oled_draw_text(&SIDE_SELECT_SIDE_NAME_FONT, "SIDE",
      1, line + 12,
      0, 0);
  oled_draw_text(&SIDE_SELECT_SIDE_NAME_FONT, side_name,
      l > 1 ? 55 : 60, line + 12,
      0, 0);

  // arrows
  if ((init_text_scroll / 4) % 2)
  {
    if (side + 1 < side_count)
      oled_draw_image(&IMAGE_CURSOR_DOWN, l > 1 ? 85 : 81, line + OLED_HEIGHT - IMAGE_CURSOR_DOWN.height, 0, 0);
    if (side > 0)
      oled_draw_image(&IMAGE_CURSOR_UP, l > 1 ? 85 : 81, line + 15, 0, 0);
  }
}

static char full_path[4096];
static char game_name[256];

FRESULT fds_side_select(char *directory, FILINFO *fno)
{
  FRESULT fr;
  uint8_t side_count, side = 0;
  int fl = strlen(fno->fname);
  int i, text_scroll = 0;

  strcpy(full_path, directory);
  strcat(full_path, "\\");
  strcat(full_path, fno->fname);
  strcpy(game_name, fno->fname);
  // trim extension
  for (i = fl - 1; i >= 0; i--)
  {
    if (game_name[i] == '.')
    {
      game_name[i] = 0;
      break;
    }
  }

  if (!fno->fsize)
  {
    fr = fds_get_side_count(full_path, &side_count, fno);
    if (fr != FR_OK) return fr;
  } else {
    if (fno->fsize % FDS_SIDE_SIZE == FDS_HEADER_SIZE)
      fno->fsize -= FDS_HEADER_SIZE;
    if (fno->fsize % FDS_SIDE_SIZE != 0)
      return FDSR_INVALID_ROM;
    side_count = fno->fsize / FDS_SIDE_SIZE;
  }
  if (side_count > 7) side_count = 7;

  if (side_count == 1)
  {
    return fds_gui_load_side(full_path, game_name, side, fno->fattrib & AM_RDO);
  }

  while (1)
  {
    fds_side_draw(side, side_count, game_name, text_scroll);
    oled_update_invisible();
    oled_switch_to_invisible();
    text_scroll++;

    if (button_up_newpress() && side > 0)
    {
      side--;
      fds_side_draw(side, side_count, game_name, text_scroll);
      oled_update_invisible();
      for (i = 0; i < OLED_HEIGHT; i++)
      {
        oled_set_line(oled_get_line() - 1);
        HAL_Delay(1);
      }
    }
    if (button_down_newpress() && side + 1 < side_count)
    {
      side++;
      fds_side_draw(side, side_count, game_name, text_scroll);
      oled_update_invisible();
      for (i = 0; i < OLED_HEIGHT; i++)
      {
        oled_set_line(oled_get_line() + 1);
        HAL_Delay(1);
      }
    }
    if (button_right_newpress())
    {
      fr = fds_gui_load_side(full_path, game_name, side, fno->fattrib & AM_RDO);
      if (fr != FR_OK) return fr;
      // back to side select
      fds_side_draw(side, side_count, game_name, text_scroll);
      oled_update_invisible();
      oled_switch_to_invisible();
    }
    if (button_left_newpress())
      return FR_OK;
    button_check_screen_off();
  }
}

DotMatrixImage* side_select_get_disk_image(uint8_t side)
{
  switch (side)
  {
  default:
  case 0:
    return (DotMatrixImage*)&IMAGE_CARD_A;
  case 1:
    return (DotMatrixImage*)&IMAGE_CARD_B;
  case 2:
    return (DotMatrixImage*)&IMAGE_CARD_C;
  case 3:
    return (DotMatrixImage*)&IMAGE_CARD_D;
  case 4:
    return (DotMatrixImage*)&IMAGE_CARD_E;
  case 5:
    return (DotMatrixImage*)&IMAGE_CARD_F;
  case 6:
    return (DotMatrixImage*)&IMAGE_CARD_G;
  case 7:
    return (DotMatrixImage*)&IMAGE_CARD_H;
  }
}
