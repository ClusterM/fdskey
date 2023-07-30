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
#include "servicemenu.h"
#include "commit.h"

void main_menu_draw(uint8_t selection)
{
  int line = oled_get_line() + OLED_HEIGHT;
  char buf[8];

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);
  // draw text
  oled_draw_text(&MAIN_MENU_FONT, "Browse disk images",
      8, line + 1,
      0, 0);
  oled_draw_text(&MAIN_MENU_FONT, "Create a blank disk",
      8, line + 11,
      0, 0);
  oled_draw_text(&MAIN_MENU_FONT, "Settings",
      8, line + 21,
      0, 0);
  // draw cursor
  oled_draw_image(&IMAGE_LARGE_CURSOR, 1, line + 2 + 10 * selection, 0, 0);

  // version number
#ifndef INTERIM
  if (!FDSKEY_VERION_SUFFIX)
    sprintf(buf, "v%d.%d", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR);
  else
    sprintf(buf, "v%d.%d%c", FDSKEY_VERION_MAJOR, FDSKEY_VERION_MINOR, FDSKEY_VERION_SUFFIX);
#else
  strcpy(buf, COMMIT);
#endif
  oled_draw_text(&MAIN_MENU_VERSION_FONT, buf,
      OLED_WIDTH - oled_get_text_length(&MAIN_MENU_VERSION_FONT, buf) - 1, line + OLED_HEIGHT - MAIN_MENU_VERSION_FONT.char_height,
      0, 0);

  oled_update_invisible();
  oled_switch_to_invisible();
//  oled_screenshot("ss_main_menu.bmp");
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
    button_left_newpress(); // remember newpress time
    if (button_left_hold_time() >= 3000)
      return MAIN_MENU_SERVICE_MENU;
    button_check_screen_off();
    HAL_Delay(1);
  }
}

void main_menu_loop()
{
  BROWSER_RESULT br;
  FILINFO selected_file;
  FRESULT fr;
  MAIN_MENU_SELECTION menu_selection = MAIN_MENU_NONE;

  if (HAL_GPIO_ReadPin(SD_DTCT_GPIO_Port, SD_DTCT_Pin))
    show_error_screen("No SD card", 1);

  fr = f_mount(&USERFatFs, "", 1);
  if (fr == FR_NO_FILESYSTEM)
  {
    show_error_screen_fr(fr, 0);
    sd_format();
    fr = f_mount(&USERFatFs, "", 1);
  }
  show_error_screen_fr(fr, 1);

  // failsafe
  // disable auto loading last state if holding left on power up
  if (button_left_holding())
  {
    fdskey_settings.remember_last_state_mode = REMEMBER_LAST_STATE_NONE;
    settings_save();
  }

  if (fdskey_settings.remember_last_state_mode == REMEMBER_LAST_STATE_NONE)
  {
    // reset state
    fdskey_settings.last_directory[0] = 0;
    fdskey_settings.last_file[0] = 0;
  } else {
    // load last filename
    strcpy(selected_file.fname, fdskey_settings.last_file);
    if (fdskey_settings.last_state != LAST_STATE_MAIN_MENU)
    {
      if (fdskey_settings.last_state == LAST_STATE_ROM)
      {
        // need to load last ROM
        show_loading_screen();
        // combine directory path and filename
        int dl = strlen(fdskey_settings.last_directory);
        int fl = strlen(fdskey_settings.last_file);
        char full_path[dl + fl + 2];
        strcpy(full_path, fdskey_settings.last_directory);
        strcat(full_path, "\\");
        strcat(full_path, fdskey_settings.last_file);
        // get file attributes
        fr = f_stat(full_path, &selected_file);
        // load ROM
        if (fr == FR_OK)
          fds_side_select(fdskey_settings.last_directory, &selected_file, 1);
      }
      // show file browser on last file
      menu_selection = MAIN_MENU_BROWSE_ROMS;
    }
  }

  while (1)
  {
    switch (menu_selection)
    {
    case MAIN_MENU_BROWSE_ROMS:
      while (1)
      {
        // show file browser
        br = browser_tree(fdskey_settings.last_directory, sizeof(fdskey_settings.last_directory), &selected_file);
        // remember last filename if need
        if (fdskey_settings.remember_last_state_mode != REMEMBER_LAST_STATE_NONE)
          strcpy(fdskey_settings.last_file, selected_file.fname);
        if (br == BROWSER_BACK || br == BROWSER_BACK_LONGPRESS)
          break;
        if (br == BROWSER_FILE)
          // load ROM
          fds_side_select(fdskey_settings.last_directory, &selected_file, 0);
        else if (br == BROWSER_FILE_LONGPRESS)
          // show file properties
          file_properties(fdskey_settings.last_directory, &selected_file);
      }
      if (fdskey_settings.remember_last_state_mode != REMEMBER_LAST_STATE_NONE)
      {
        // remember last state as main menu
        fdskey_settings.last_state = LAST_STATE_MAIN_MENU;
        settings_save();
      }
      break;
    case MAIN_MENU_NEW_ROM:
      // new ROM dialog
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
      // settings dialog
      settings_menu();
      break;
    case MAIN_MENU_SERVICE_MENU:
      // service menu
      service_menu();
      menu_selection = 0;
      break;
    default:
      break;
    }
    menu_selection = main_menu(menu_selection);
  }
}
