#include <stdio.h>
#include "mainmenu.h"
#include "main.h"
#include "oled.h"
#include "buttons.h"

void main_menu_draw(uint8_t selection)
{
  int line = oled_get_line() + OLED_HEIGHT;
  char buf[8];

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  // draw text
  oled_draw_text(&MAIN_MENU_FONT, "Browse ROMs",
      10, line + 1,
      0, 0);
  oled_draw_text(&MAIN_MENU_FONT, "Create blank disk",
      10, line + 11,
      0, 0);
  oled_draw_text(&MAIN_MENU_FONT, "Settings",
      10, line + 21,
      0, 0);
  // draw cursor
  oled_draw_image(&IMAGE_LARGE_CURSOR, 3, line + 2 + 10 * selection, 0, 0);

  // version number
  sprintf(buf, "v%d.%d", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR);
  oled_draw_text(&MAIN_MENU_VERSION_FONT, buf,
      OLED_WIDTH - oled_get_text_length(&MAIN_MENU_VERSION_FONT, buf), line + OLED_HEIGHT - MAIN_MENU_VERSION_FONT.char_height + 1,
      0, 0);

  oled_update_invisible();
  oled_switch_to_invisible();
}

uint8_t main_menu(uint8_t selection)
{
  if (selection > 2) selection = 0;

  main_menu_draw(selection);

  while (1)
  {
    if (button_up_newpress() && selection > 0)
    {
      selection--;
      main_menu_draw(selection);
    }
    if (button_down_newpress() && selection < 2)
    {
      selection++;
      main_menu_draw(selection);
    }
    if (button_right_newpress())
      return selection;
    button_check_screen_off();
    HAL_Delay(1);
  }
}
