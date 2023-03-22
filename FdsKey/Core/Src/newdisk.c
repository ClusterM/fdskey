#include <string.h>
#include "newdisk.h"
#include "oled.h"
#include "app_fatfs.h"
#include "buttons.h"
#include "fdsemu.h"
#include "settings.h"
#include "splash.h"

static FRESULT new_disk_create(char *filename, int sides)
{
  int i;
  FRESULT fr;
  FIL fp;
  uint8_t buff[500];
  UINT bw;

  show_saving_screen();

  memset(buff, 0, sizeof(buff));

  fr = f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
    return fr;
  // just fill file with zeros
  for (i = 0; i < FDS_SIDE_SIZE * sides; i += sizeof(buff))
  {
    fr = f_write(&fp, buff, sizeof(buff), &bw);
    if (fr != FR_OK)
    {
      f_close(&fp);
      return fr;
    }
  }
  fr = f_close(&fp);

  if (fr == FR_OK)
    show_message("File successfully created");

  return fr;
}

static void new_disk_select_sides_count_draw(int sides, int blink)
{
  int line = oled_get_line() + OLED_HEIGHT;
  char title[] = "How many disk sides?";
  char number[] = "0";

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);

  // title
  oled_draw_text(&FONT_STANDARD_6, title,
      OLED_WIDTH / 2 - oled_get_text_length(&FONT_STANDARD_6, title) / 2, line + 0,
      0, 0);

  // number
  number[0] += sides;
  oled_draw_text(&FONT_GAMEGIRL_CLASSIC_12, number,
      OLED_WIDTH / 2 - oled_get_text_length(&FONT_GAMEGIRL_CLASSIC_12, number) / 2, line + 14,
      0, 0);

  // arrows
  if ((blink / 4) % 2)
  {
    if (sides < 8)
      oled_draw_image(&IMAGE_CURSOR_UP_W, OLED_WIDTH / 2 - IMAGE_CURSOR_UP_W.width / 2,
          line + 12, 0, 0);
    if (sides > 1)
      oled_draw_image(&IMAGE_CURSOR_DOWN_W, OLED_WIDTH / 2 - IMAGE_CURSOR_DOWN_W.width / 2,
          line + 29, 0, 0);
  }

  oled_update_invisible();
  oled_switch_to_invisible();
}

static FRESULT new_disk_select_sides_count(char* filename)
{
  int sides = 1;
  int blink = 0;

  while (1)
  {
    if (button_up_newpress() && sides < 8)
      sides++;
    if (button_down_newpress() && sides > 1)
          sides--;
    if (button_left_newpress())
      return FDSR_CANCELLED; // cancel
    if (button_right_newpress())
      return new_disk_create(filename, sides);
    new_disk_select_sides_count_draw(sides, blink);
    button_check_screen_off();
    blink++;
  }
  return FR_OK;
}

static void new_disk_filename_draw(char *filename, int blink)
{
  int i, l;
  char f_filename[NEW_DISK_FILENAME_LENGTH + 4 + 1];
  int line = oled_get_line() + OLED_HEIGHT;
  char title[] = "Enter filename";

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);

  // title
  oled_draw_text(&FONT_STANDARD_6, title,
      OLED_WIDTH / 2 - oled_get_text_length(&FONT_STANDARD_6, title) / 2, line + 0,
      0, 0);

  l = strlen(filename);
  strcpy(f_filename, filename);
  for (i = 0; i < NEW_DISK_FILENAME_LENGTH; i++)
  {
    // spaces to underscores
    if (i < l)
    {
      if (f_filename[i] == ' ')
        f_filename[i] = '_';
    } else {
      // blank symbols
      f_filename[i] = '_';
    }
  }
  f_filename[NEW_DISK_FILENAME_LENGTH] = 0;
  strcat(f_filename, ".fds");

  // draw filename
  oled_draw_text(&FONT_GAMEGIRL_CLASSIC_6, f_filename,
      8, line + 16,
      0, 0);

  // arrows
  if ((blink / 4) % 2)
  {
    oled_draw_image(&IMAGE_CURSOR_UP_W, 1 + l * (FONT_GAMEGIRL_CLASSIC_6.char_width + FONT_GAMEGIRL_CLASSIC_6.spacing),
        line + 13, 0, 0);
    oled_draw_image(&IMAGE_CURSOR_DOWN_W, 1 + l * (FONT_GAMEGIRL_CLASSIC_6.char_width + FONT_GAMEGIRL_CLASSIC_6.spacing),
        line + 25, 0, 0);
  }

  oled_update_invisible();
  oled_switch_to_invisible();
}

static uint8_t is_forbidden_char(char c)
{
  return c == '\\' || c == '/' || c == ':' || c == '*' || c ==  '?' || c == '"' || c == '<' || c == '>' || c == '|';
}

FRESULT new_disk()
{
  char filename[NEW_DISK_FILENAME_LENGTH + 1] = "NEW";
  char filename_trimmed[NEW_DISK_FILENAME_LENGTH + 4 + 1];
  int blink = 0;
  int i, l;
  FRESULT fr;

  l = strlen(filename);
  while (1)
  {
    if (button_up_newpress())
    {
      // character plus
      do
      {
        filename[l - 1]++;
        if (filename[l - 1] > NEW_DISK_FILENAME_END_CHAR)
          filename[l - 1] = NEW_DISK_FILENAME_START_CHAR;
      } while (is_forbidden_char(filename[l - 1]) || (l == 1 && filename[0] == ' '));
    }
    if (button_down_newpress())
    {
      // character minus
      do
      {
        filename[l - 1]--;
        if (filename[l - 1] < NEW_DISK_FILENAME_START_CHAR)
          filename[l - 1] = NEW_DISK_FILENAME_END_CHAR;
      } while (is_forbidden_char(filename[l - 1]) || (l == 1 && filename[0] == ' '));
    }
    if (button_right_newpress())
    {
      if (l < NEW_DISK_FILENAME_LENGTH)
      {
        // add new char (space)
        strcat(filename, " ");
        l++;
      } else {
        // commit
        strcpy(filename_trimmed, filename);
        for (i = l - 1; i > 0; i--)
        {
          // delete trailing spaces
          if (filename_trimmed[i] == ' ')
            filename_trimmed[i] = 0;
          else
            break;
        }
        strcat(filename_trimmed, ".fds");
        fr = new_disk_select_sides_count(filename_trimmed);
        if (fr != FDSR_CANCELLED) {
          if (fr == FR_OK)
          {
            // select file in browser
            fdskey_settings.last_directory[0] = 0;
            strcpy(fdskey_settings.last_file, filename_trimmed);
          }
          return fr;
        }
      }
    }
    if (button_left_newpress())
    {
      if (l > 1)
      {
        // delete last character
        filename[l - 1] = 0;
        l--;
      } else {
        // cancel
        return FDSR_CANCELLED;
      }
    }
    new_disk_filename_draw(filename, blink);
    button_check_screen_off();
    blink++;
  }
}
