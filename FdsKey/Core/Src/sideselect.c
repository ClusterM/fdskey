#include <fdsemugui.h>
#include <string.h>
#include <stdio.h>
#include "sideselect.h"
#include "fdsemu.h"
#include "oled.h"
#include "app_fatfs.h"
#include "splash.h"
#include "buttons.h"
#include "settings.h"

static void fds_side_draw(uint8_t side, uint8_t side_count, char* game_name, int text_scroll)
{
  char side_name[8];
  DotMatrixImage *disk_image;
  int max_width, text_width, total_scroll;
  int line = oled_get_line() + OLED_HEIGHT;
  int l;

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);

  if (side_count <= 2)
  {
    side_name[0] = 'A' + side;
    side_name[1] = 0;
  } else {
    sprintf(side_name, "%d%c", side / 2 + 1, 'A' + (side % 2));
  }
  l = strlen(side_name);

  disk_image = side_select_get_disk_image(side, side_count);

  // arrows
  if ((text_scroll / 8) % 2)
  {
    if (side + 1 < side_count)
      oled_draw_image(&IMAGE_CURSOR_DOWN, l > 1 ? 85 : 81, line + OLED_HEIGHT - IMAGE_CURSOR_DOWN.height, 0, 0);
    if (side > 0)
      oled_draw_image(&IMAGE_CURSOR_UP, l > 1 ? 85 : 81, line + 16, 0, 0);
  }

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
}

void fds_side_select(char *directory, FILINFO *fno, uint8_t load_first)
{
  FRESULT fr;
  uint8_t side_count, side = 0;
  char game_name[256];
  int dl = strlen(directory);
  int fl = strlen(fno->fname);
  char full_path[dl + fl + 2];
  int i, text_scroll = 0;

  // save state if need
  if (fdskey_settings.remember_last_state_mode == REMEMBER_LAST_STATE_ROM
      && fdskey_settings.last_state != LAST_STATE_ROM)
  {
    fdskey_settings.last_state = LAST_STATE_ROM;
    settings_save();
  }

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

  if (fno->fsize % FDS_ROM_SIDE_SIZE == FDS_ROM_HEADER_SIZE)
    fno->fsize -= FDS_ROM_HEADER_SIZE;
  if (fno->fsize % FDS_ROM_SIDE_SIZE != 0)
  {
    show_error_screen_fr(FDSR_INVALID_ROM, 0);
    return;
  }
  side_count = fno->fsize / FDS_ROM_SIDE_SIZE;
  if (!side_count)
  {
    // empty ROM
    show_error_screen_fr(FDSR_INVALID_ROM, 0);
    return;
  }

  if (side_count == 1)
  {
    // Single sided ROM, do not show side select dialog
    fr = fds_gui_load_side(full_path, game_name, side, side_count, fno->fattrib & AM_RDO);
    show_error_screen_fr(fr, fr < 0x80);
    return;
  }

  if (load_first)
  {
    // need to load first side
    fr = fds_gui_load_side(full_path, game_name, 0, side_count, fno->fattrib & AM_RDO);
    show_error_screen_fr(fr, fr < 0x80);
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
      fr = fds_gui_load_side(full_path, game_name, side, side_count, fno->fattrib & AM_RDO);
      show_error_screen_fr(fr, fr < 0x80);
      // back to side select
      fds_side_draw(side, side_count, game_name, text_scroll);
      oled_update_invisible();
      oled_switch_to_invisible();
    }
    if (button_left_newpress())
      return;
    button_check_screen_off();
  }
}

DotMatrixImage* side_select_get_disk_image(uint8_t side, uint8_t side_count)
{
  if (side_count <= 1)
    return (DotMatrixImage*)&IMAGE_CARD_SINGLE;
  switch (side)
  {
  case 0:
    if (side_count <= 2)
      return (DotMatrixImage*)&IMAGE_CARD_A;
    else
      return (DotMatrixImage*)&IMAGE_CARD_1A;
  case 1:
    if (side_count <= 2)
      return (DotMatrixImage*)&IMAGE_CARD_B;
    else
      return (DotMatrixImage*)&IMAGE_CARD_1B;
  case 2:
    return (DotMatrixImage*)&IMAGE_CARD_2A;
  case 3:
    return (DotMatrixImage*)&IMAGE_CARD_2B;
  case 4:
    return (DotMatrixImage*)&IMAGE_CARD_3A;
  case 5:
    return (DotMatrixImage*)&IMAGE_CARD_3B;
  case 6:
    return (DotMatrixImage*)&IMAGE_CARD_4A;
  case 7:
    return (DotMatrixImage*)&IMAGE_CARD_4B;
  default:
    return (DotMatrixImage*)&IMAGE_CARD_UNKNOWN;
  }
}
