#include <stdint.h>
#include <string.h>
#include "fileproperties.h"
#include "oled.h"
#include "app_fatfs.h"
#include "buttons.h"
#include "splash.h"
#include "confirm.h"

static void file_properties_draw(uint8_t selection, uint8_t wp)
{
  int line = oled_get_line() + OLED_HEIGHT;
  char* off = "\x86";
  char* on = "\x87";
  const int y_offset = 2;
  const int x_offset = 15;

  // clear screen
  oled_draw_rectangle(0, line, OLED_WIDTH - 1, line + OLED_HEIGHT - 1, 1, 0);

  // draw menu items
  oled_draw_text(&FONT_SLIMFONT_8, "Write protect",
      IMAGE_MEDIUM_CURSOR.width + x_offset + 2, line + 2,
      0, 0);
  oled_draw_text(&FONT_SLIMFONT_8, wp ? on : off,
      OLED_WIDTH - FONT_SLIMFONT_8.char_width - x_offset, line + y_offset,
      0, 0);
  oled_draw_text(&FONT_SLIMFONT_8, "Restore backup",
      IMAGE_MEDIUM_CURSOR.width + x_offset + 2, line + 10 + y_offset,
      0, 0);
  oled_draw_text(&FONT_SLIMFONT_8, "Delete file",
      IMAGE_MEDIUM_CURSOR.width + x_offset + 2, line + 20 + y_offset,
      0, 0);

  // cursor
  oled_draw_image(&IMAGE_MEDIUM_CURSOR, x_offset, line + 10 * selection + y_offset + 1, 0, 0);

  oled_update_invisible();
  oled_switch_to_invisible();
}

FRESULT file_write_protect(char *path, uint8_t rdo)
{
  show_saving_screen();
  return f_chmod(path, rdo ? AM_RDO : 0, AM_RDO);
}

FRESULT file_restore_backup(char *path)
{
  FILINFO fno;
  FRESULT fr;
  FIL fp, fp_backup;
  char backup_path[strlen(path) + 4];
  uint8_t buff[4096];
  UINT br, bw;

  show_loading_screen();

  strcpy(backup_path, path);
  strcat(backup_path, ".bak");

  fr = f_stat(backup_path, &fno);
  if (fr == FR_NO_FILE)
  {
    show_message("There is no backup", 1);
    return FR_OK;
  }
  if (fr != FR_OK) return fr;

  if (!confirm("Restore backup?"))
    return FR_OK;

  show_saving_screen();
  fr = f_open(&fp_backup, backup_path, FA_READ);
  if (fr != FR_OK) return fr;
  fr = f_open(&fp, path, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
  {
    f_close(&fp);
    return fr;
  }

  do
  {
    fr = f_read(&fp_backup, buff, sizeof(buff), &br);
    if (fr != FR_OK)
    {
      f_close(&fp);
      f_close(&fp_backup);
      return fr;
    }
    fr = f_write(&fp, buff, br, &bw);
    if (bw != br)
    {
      f_close(&fp);
      f_close(&fp_backup);
      return fr;
    }
  } while (br > 0);
  f_close(&fp);
  f_close(&fp_backup);

  show_message("Backup restored", 1);
  return FR_OK;
}

FRESULT file_delete(char *path)
{
  if (!confirm("Delete file?"))
    return FR_OK;
  show_saving_screen();
  return f_unlink(path);
}

void file_properties(char *directory, FILINFO *fno)
{
  FRESULT fr;
  uint8_t selection = 0;
  int dl = strlen(directory);
  int fl = strlen(fno->fname);
  char full_path[dl + fl + 2];

  strcpy(full_path, directory);
  strcat(full_path, "\\");
  strcat(full_path, fno->fname);

  file_properties_draw(selection, fno->fattrib & AM_RDO);
  while (1)
  {
    if (button_up_newpress() && selection > 0)
    {
      selection--;
      file_properties_draw(selection, fno->fattrib & AM_RDO);
    }
    if (button_down_newpress() && selection < 2)
    {
      selection++;
      file_properties_draw(selection, fno->fattrib & AM_RDO);
    }
    if (button_right_newpress())
    {
      switch(selection)
      {
      case FILE_PROPERTIES_WRITE_PROTECT:
        fr = file_write_protect(full_path, !(fno->fattrib & AM_RDO));
        if (fr == FR_OK)
          fno->fattrib ^= AM_RDO;
        show_error_screen_fr(fr, 0);
        break;
      case FILE_PROPERTIES_RESTORE_BACKUP:
        fr = file_restore_backup(full_path);
        show_error_screen_fr(fr, 0);
        break;
      case FILE_PROPERTIES_DELETE:
        fr = file_delete(full_path);
        show_error_screen_fr(fr, 0);
        return;
      }
      file_properties_draw(selection, fno->fattrib & AM_RDO);
    }
    if (button_left_newpress())
      return;

    button_check_screen_off();
    HAL_Delay(1);
  }
}
