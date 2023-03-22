#include "main.h"
#include "oled.h"
#include "buttons.h"

static void confirm_draw(char *text, uint8_t selection)
{
  int line = oled_get_line() + OLED_HEIGHT;

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  // title
  oled_draw_text(&FONT_STANDARD_6, text,
      OLED_WIDTH / 2 - oled_get_text_length(&FONT_STANDARD_6, text) / 2, line + 0,
      0, 0);

  oled_draw_text(&FONT_GAMEGIRL_CLASSIC_6, "NO",
      OLED_WIDTH / 2 - 12, line + 11,
      0, 0);
  oled_draw_text(&FONT_GAMEGIRL_CLASSIC_6, "YES",
      OLED_WIDTH / 2 - 12, line + 21,
      0, 0);

  oled_draw_image(&IMAGE_MEDIUM_CURSOR, OLED_WIDTH / 2 - 18, line + 12 + 10 * selection, 0, 0);

  oled_update_invisible();
  oled_switch_to_invisible();
}

uint8_t confirm(char *text)
{
  uint8_t selection = 0;
  confirm_draw(text, selection);
  while (1)
  {
    if (button_up_newpress())
    {
      selection = 0;
      confirm_draw(text, selection);
    }
    if (button_down_newpress())
    {
      selection = 1;
      confirm_draw(text, selection);
    }
    if (button_left_newpress())
      return 0;
    if (button_right_newpress())
      return selection;
    button_check_screen_off();
    HAL_Delay(1);
  }
}
