#include <string.h>
#include "fdsemu.h"
#include "oled.h"
#include "fatfs.h"
#include "browser.h"
#include "splash.h"
#include "buttons.h"

static void fds_side_draw(uint8_t side, int line, char* game_name, int text_scroll)
{
  char *text;
  DotMatrixImage *image;
  int max_width, text_width, total_scroll;

  switch (side)
  {
  case 0:
    text = "SIDE A";
    image = (DotMatrixImage*)&IMAGE_CARD_A;
    break;
  case 1:
    text = "SIDE B";
    image = (DotMatrixImage*)&IMAGE_CARD_B;
    break;
  case 2:
    text = "SIDE C";
    image = (DotMatrixImage*)&IMAGE_CARD_C;
    break;
  case 3:
    text = "SIDE D";
    image = (DotMatrixImage*)&IMAGE_CARD_D;
    break;
  case 4:
    text = "SIDE E";
    image = (DotMatrixImage*)&IMAGE_CARD_E;
    break;
  case 5:
    text = "SIDE F";
    image = (DotMatrixImage*)&IMAGE_CARD_F;
    break;
  case 6:
    text = "SIDE G";
    image = (DotMatrixImage*)&IMAGE_CARD_G;
    break;
  case 7:
    text = "SIDE H";
    image = (DotMatrixImage*)&IMAGE_CARD_H;
    break;
  }

  max_width = OLED_WIDTH - image->width - 4;
  text_width = oled_get_text_length(&BROWSER_FONT, game_name) - 1 /*spacing*/;
  if (text_width > max_width)
  {
    total_scroll = BROWSER_HORIZONTAL_SCROLL_PAUSE + (text_width - max_width) + BROWSER_HORIZONTAL_SCROLL_PAUSE;
    text_scroll /= BROWSER_HORIZONTAL_SCROLL_SPEED;
    text_scroll %= total_scroll * 2;
    // two-directional
    if (text_scroll > total_scroll)
      text_scroll = total_scroll * 2 - text_scroll;
    if (text_scroll < BROWSER_HORIZONTAL_SCROLL_PAUSE)
      text_scroll = 0;
    else if (text_scroll >= BROWSER_HORIZONTAL_SCROLL_PAUSE + (text_width - max_width))
      text_scroll = text_width - max_width;
    else
      text_scroll = text_scroll - BROWSER_HORIZONTAL_SCROLL_PAUSE;
  } else {
    text_scroll = 0;
  }

  oled_draw_rectangle(0, line, OLED_WIDTH - 1, OLED_HEIGHT - 1 + line, 1, 0);
  oled_draw_text_cropped(&BROWSER_FONT, game_name,
      1, line,
      text_scroll, max_width,
      0, 0,
      0, 0);
  oled_draw_text(&FONT_HAETTENSCHWEILER_18, text,
      1, 7 + line,
      0, 0);
  oled_draw_image(
      image, OLED_WIDTH - image->width - 1, 1 + line,
      0, 0);
}

FRESULT fds_side_select(char *directory, char *filename)
{
  show_loading_screen();

  FRESULT fr;
  uint8_t side_count, side = 0;
  FILINFO fileinfo;
  int fl = strlen(filename);
  char full_path[strlen(directory) + 1 + fl + 1];
  char game_name[fl + 1];
  int i, text_scroll = 0;

  strcpy(full_path, directory);
  strcat(full_path, "\\");
  strcat(full_path, filename);
  strcpy(game_name, filename);
  // trim extension
  for (i = fl - 1; i >= 0; i--)
  {
    if (game_name[i] == '.')
    {
      game_name[i] = 0;
      break;
    }
  }

  fr = fds_get_side_count(full_path, &side_count, &fileinfo);
  if (fr != FR_OK) return fr;
  if (side_count > 7) side_count = 7;

  while (1)
  {
    fds_side_draw(side, oled_get_line() + OLED_HEIGHT, game_name, text_scroll);
    oled_update_invisible();
    //oled_update_full();
    oled_switch_to_invisible();
    text_scroll++;

    if (button_up_newpress() && side > 0)
    {
      side--;
      fds_side_draw(side, oled_get_line() + OLED_HEIGHT, game_name, text_scroll);
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
      fds_side_draw(side, oled_get_line() + OLED_HEIGHT, game_name, text_scroll);
      oled_update_invisible();
      for (i = 0; i < OLED_HEIGHT; i++)
      {
        oled_set_line(oled_get_line() + 1);
        HAL_Delay(1);
      }
    }
    if (button_left_newpress())
      return FR_OK;
    //HAL_Delay(1);
  }
}
