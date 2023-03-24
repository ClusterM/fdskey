#include <string.h>
#include <stdio.h>
#include "mainmenu.h"
#include "main.h"
#include "oled.h"
#include "buttons.h"
#include "sdcard.h"
#include "fdsemu.h"
#include "browser.h"
#include "splash.h"
#include "sideselect.h"
#include "settings.h"
#include "newdisk.h"
#include "fileproperties.h"

void main_menu_draw(uint8_t selection)
{
  int line = oled_get_line() + OLED_HEIGHT;
  char buf[8];

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  // draw text
  oled_draw_text(&MAIN_MENU_FONT, "Browse disk images",
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
      OLED_WIDTH - oled_get_text_length(&MAIN_MENU_VERSION_FONT, buf) - 1, line + OLED_HEIGHT - MAIN_MENU_VERSION_FONT.char_height,
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

void main_menu_loop()
{
  BROWSER_RESULT br;
  FILINFO selected_file;
  FATFS FatFs;
  FRESULT fr;
  uint8_t menu_selection = 0xFF;

  while (HAL_GPIO_ReadPin(SD_DTCT_GPIO_Port, SD_DTCT_Pin))
    show_error_screen("No SD card", 0);
  HAL_StatusTypeDef r = SD_init();
  if (r != HAL_OK)
    show_error_screen("Can't init SD card", 1);
  fr = f_mount(&FatFs, "", 1);
  show_error_screen_fr(fr, 1);

  if (!fdskey_settings.remember_last_file)
  {
    fdskey_settings.last_directory[0] = 0;
    fdskey_settings.last_file[0] = 0;
  } else {
    strcpy(selected_file.fname, fdskey_settings.last_file);
    if (!fdskey_settings.last_state_menu)
      menu_selection = MAIN_MENU_BROWSE_ROMS;
  }

  while (1)
  {
    switch (menu_selection)
    {
    case MAIN_MENU_BROWSE_ROMS:
      while (1)
      {
        fr = browser_tree(fdskey_settings.last_directory, sizeof(fdskey_settings.last_directory), &selected_file, &br);
        show_error_screen_fr(fr, 1);
        // remember last directory (root) and file
        strcpy(fdskey_settings.last_file, selected_file.fname);
        if (br == BROWSER_BACK)
          break;
        fdskey_settings.last_state_menu = 0;
        settings_save();
        if (br == BROWSER_FILE)
          // load ROM
          fds_side_select(fdskey_settings.last_directory, &selected_file);
        else if (br == BROWSER_FILE_LONGPRESS)
          file_properties(fdskey_settings.last_directory, &selected_file);
      }
      fdskey_settings.last_state_menu = 1; // remember last state as main menu
      settings_save();
      break;
    case MAIN_MENU_NEW_ROM:
      fr = new_disk();
      if (fr != FDSR_CANCELLED)
      {
        if (fr == FR_OK)
        {
         // show browser and select file
         strcpy(selected_file.fname, fdskey_settings.last_file);
         menu_selection = MAIN_MENU_BROWSE_ROMS;
         continue;
        } else {
          show_error_screen_fr(fr, 0);
        }
      }
      break;
    case MAIN_MENU_SETTINGS:
      settings_menu();
      break;
    }
    menu_selection = main_menu(menu_selection);
  }
}
