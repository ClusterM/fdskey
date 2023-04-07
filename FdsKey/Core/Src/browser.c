#include <string.h>
#include "main.h"
#include "browser.h"
#include "app_fatfs.h"
#include "oled.h"
#include "buttons.h"
#include "splash.h"
#include "fdsemu.h"
#include "settings.h"

static DYN_FILINFO** dir_list = 0;
static DYN_FILINFO** file_list = 0;
static int dir_count = 0;
static int file_count = 0;

static void browser_free();

#ifdef BROWSER_USE_RUSSIAN
static void cp866to1251(char *text)
{
  while (*text)
  {
    if (*text >= 0x80 && *text <= 0xAF) // А-Я, а-п
      *text += 0x40;
    else if (*text >= 0xE0 && *text <= 0xEF) // р-я
      *text += 0x10;
    else if (*text == 0xF0) // Ё
      *text = 0xA8;
    else if (*text == 0xF1) // ё
      *text = 0xB8;
    text++;
  }
}

static void cp1251to866(char *text)
{
  while (*text)
  {
    if (*text >= 0xC0 && *text <= 0xEF) // А-Я, а-п
      *text -= 0x40;
    else if (*text >= 0xF0 && *text <= 0xFF) // р-я
      *text -= 0x10;
    else if (*text == 0xA8) // Ё
      *text = 0xF0;
    else if (*text == 0xB8) // ё
      *text = 0xF1;
    text++;
  }
}
#endif

// Merge sort
// left source half is A[iBegin:iMiddle-1]
// right source half is A[iMiddle:iEnd-1]
// result is B[iBegin:iEnd-1]
static void top_down_merge(DYN_FILINFO** A, int iBegin, int iMiddle, int iEnd, DYN_FILINFO** B)
{
  int i = iBegin, j = iMiddle, k;

  // While there are elements in the left or right runs...
  for (k = iBegin; k < iEnd; k++) {
    // If left run head exists and is <= existing right run head.
    if (i < iMiddle && (j >= iEnd || strcasecmp(A[i]->filename, A[j]->filename) <= 0)) {
      B[k] = A[i];
      i = i + 1;
    } else {
      B[k] = A[j];
      j = j + 1;
    }
  }
}
// split A[] into 2 runs, sort both runs into B[], merge both runs from B[] to A[]
// iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
static void top_down_split_merge(DYN_FILINFO** B, int iBegin, int iEnd, DYN_FILINFO** A)
{
  if (iEnd - iBegin <= 1)                     // if run size == 1
      return;                                 //   consider it sorted
  // split the run longer than 1 item into halves
  int iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
  // recursively sort both runs from array A[] into B[]
  top_down_split_merge(A, iBegin,  iMiddle, B);  // sort the left  run
  top_down_split_merge(A, iMiddle,    iEnd, B);  // sort the right run
  // merge the resulting runs from array B[] into A[]
  top_down_merge(B, iBegin, iMiddle, iEnd, A);
}
// start sort
static void top_down_merge_sort(DYN_FILINFO** A, int n)
{
  DYN_FILINFO* B[n];
  memcpy(B, A, sizeof(DYN_FILINFO*) * n);
  top_down_split_merge(B, 0, n, A); // sort data from B[] into A[]
}

// draw single menu item
static void browser_draw_item(uint8_t line, int item, uint8_t is_selected, int text_scroll)
{
  uint8_t is_dir;
  int i, offset, max_width, text_width, total_scroll;
  char* text;
  if (item < dir_count)
    text = dir_list[item]->filename;
  else if (item < dir_count + file_count)
  {
    text = file_list[item - dir_count]->filename;
    // hide extension if enabled and .fds file
    if (fdskey_settings.hide_extensions && (strcasecmp(text + strlen(text) - 4, ".fds") == 0))
    {
      char trimmed[_MAX_LFN + 1];
      strncpy(trimmed, text, _MAX_LFN);
      text = trimmed;
      for (i = strlen(text) - 1; i >= 0; i--)
      {
        if (text[i] == '.')
        {
          text[i] = 0;
          break;
        }
      }
    }
  }
  else
    text = "";

#ifdef BROWSER_USE_RUSSIAN
  // temporary convert to cp1251
  cp866to1251(text);
#endif

  is_dir = item < dir_count;
  oled_draw_rectangle(0, line * 8, OLED_WIDTH - 1, line * 8 + 7, 1, 0);
  if (is_selected)
    oled_draw_image(&IMAGE_CURSOR, 0, line * 8, 0, 0);
  offset = IMAGE_CURSOR.width + (is_dir ? BROWSER_FOLDER_IMAGE.width + 1: 0);
  max_width = OLED_WIDTH - offset - 1;
  if (is_dir)
    oled_draw_image(&BROWSER_FOLDER_IMAGE, IMAGE_CURSOR.width, line * 8, 0, 0);

  text_width = oled_get_text_length(&BROWSER_FONT, text) - 1 /*spacing*/;
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

  oled_draw_text_cropped(&BROWSER_FONT, text,
      offset, line * 8,
      text_scroll, max_width,
      0, 0,
      0, 0);
  oled_update(line, line);

#ifdef BROWSER_USE_RUSSIAN
  // convert back
  cp1251to866(text);
#endif
}

// show single directory menu
// selection - start selection
// is_selected - non-zero value written if right button pressed
// returns selected item id
static int browser_menu(int selection, uint8_t *is_selected)
{
  int i;
  int text_scroll = 0;
  int item_count = dir_count + file_count;
  int line = selection - 2;
  if (line + 4 > item_count) line = item_count - 4;
  if (line < 0) line = 0;

  for (i = 0; i < 4; i++)
  {
    browser_draw_item((oled_get_line() + OLED_HEIGHT) / 8 + i, line + i, line + i == selection, 0);
  }
  oled_switch_to_invisible();

  while (1)
  {
    if (button_up_newpress() && selection > 0)
    {
      browser_draw_item(oled_get_line() / 8 + selection - line, selection, 0, 0);
      selection--;
      browser_draw_item(oled_get_line() / 8 + selection - line, selection, 1, 0);
      text_scroll = 0;
    }
    if (button_down_newpress() && selection + 1 < item_count)
    {
      browser_draw_item(oled_get_line() / 8 + selection - line, selection, 0, 0);
      selection++;
      browser_draw_item(oled_get_line() / 8 + selection - line, selection, 1, 0);
      text_scroll = 0;
    }
    if (button_left_newpress()) {
      *is_selected = 0;
      return selection; // back
    }
    if (button_right_newpress()) {
      *is_selected = 1;
      return selection;
    }
    while (selection < line && line)
    {
      line--;
      for (i = 0; i < 8; i++) {
        oled_set_line(oled_get_line() - 1);
        HAL_Delay(5);
      }
    }
    while (selection > line + 3)
    {
      line++;
      for (i = 0; i < 8; i++) {
        oled_set_line(oled_get_line() + 1);
        HAL_Delay(5);
      }
    }
    browser_draw_item(oled_get_line() / 8 + selection - line, selection, 1, text_scroll);
    text_scroll++;
    button_check_screen_off();
    HAL_Delay(1);
  }
}

// load single directory
// path - directory
// output - output file/directory info
// result - output result
// select - default item to select
FRESULT browser(char *path, FILINFO *output, BROWSER_RESULT *result, char *select)
{
  FRESULT fr;
  DIR dir;
  FILINFO fno;
  int mem_dir_count = 512;
  int mem_file_count = 512;
  int i, r, selection;
  uint8_t is_selected = 0;
  dir_count = 0;
  file_count = 0;

  show_loading_screen();

  dir_list = malloc(mem_dir_count * sizeof(char*));
  if (!dir_list)
    return FDSR_OUT_OF_MEMORY;
  file_list = malloc(mem_file_count * sizeof(char*));
  if (!file_list)
    return FDSR_OUT_OF_MEMORY;

  // load file and directory names
  fr = f_opendir(&dir, path);
  if (fr != FR_OK) {
    browser_free();
    return fr;
  }
  while (1)
  {
    fr = f_readdir(&dir, &fno);
    if (fr != FR_OK)
    {
      browser_free();
      return fr;
    }
    if (!fno.fname[0])
      break;
    if (!fdskey_settings.hide_hidden || !(fno.fattrib & AM_HID))
    {
      if (fno.fattrib & AM_DIR)
      {
        // directory
        if (dir_count + 1 > mem_dir_count)
        {
          // reallocate memory for directories list
          mem_dir_count *= 2;
          dir_list = realloc(dir_list, mem_dir_count * sizeof(char*));
          if (!dir_list)
            return FDSR_OUT_OF_MEMORY;
        }
        // allocate memory for new directory entry
        dir_list[dir_count] = malloc(sizeof(DYN_FILINFO));
        if (!dir_list[dir_count]) return FDSR_OUT_OF_MEMORY;
        dir_list[dir_count]->filename = malloc(strlen(fno.fname) + 1);
        if (!dir_list[dir_count]->filename) return FDSR_OUT_OF_MEMORY;
        strcpy(dir_list[dir_count]->filename, fno.fname);
        dir_count++;
      } else {
        if (fdskey_settings.hide_non_fds)
        {
          if (strcasecmp(fno.fname + strlen(fno.fname) - 4, ".fds") != 0)
              continue;
        }
        if (file_count + 1 > mem_file_count)
        {
          // reallocate memory for file list
          mem_file_count *= 2;
          file_list = realloc(file_list, mem_file_count * sizeof(char*));
          if (!file_list)
            return FDSR_OUT_OF_MEMORY;
        }
        // allocate memory for file entry
        file_list[file_count] = malloc(sizeof(DYN_FILINFO));
        if (!file_list[file_count]) return FDSR_OUT_OF_MEMORY;
        file_list[file_count]->filename = malloc(strlen(fno.fname) + 1);
        if (!file_list[file_count]->filename) return FDSR_OUT_OF_MEMORY;
        strcpy(file_list[file_count]->filename, fno.fname);
        file_list[file_count]->fsize = fno.fsize;
        file_list[file_count]->fattrib = fno.fattrib;
        file_count++;
      }
    }
  }

  // show_free_memory();

  f_closedir(&dir);
  // free some memory (do i readlly need it?)
  dir_list = realloc(dir_list, dir_count * sizeof(char*));
  if (dir_count && !dir_list) return FDSR_OUT_OF_MEMORY;
  file_list = realloc(file_list, file_count * sizeof(char*));
  if (file_count && !file_list) return FDSR_OUT_OF_MEMORY;

  if (!dir_count && !file_count)
  {
    // nothing to show
    browser_free();
    show_message("The directory is empty", 1);
    *result = BROWSER_BACK;
    return FR_OK;
  }

  // sort them
  top_down_merge_sort(dir_list, dir_count);
  top_down_merge_sort(file_list, file_count);

  selection = 0;
  if (select && select[0])
  {
    // find default selection entry
    for (i = 0; i < dir_count + file_count; i++)
    {
      char* name = i < dir_count ? dir_list[i]->filename : file_list[i - dir_count]->filename;
      if (strcmp(name, select) == 0)
      {
        selection = i;
        break;
      }
    }
  }

  r = browser_menu(selection, &is_selected);
  if (r < dir_count)
  {
    *result = BROWSER_DIRECTORY;
    strncpy(output->fname, dir_list[r]->filename, sizeof(output->fname));
    output->fname[sizeof(output->fname) - 1] = 0;
  } else {
    *result = BROWSER_FILE;
    while (button_right_holding())
    {
      if (button_right_hold_time() >= BROWSER_LONGPRESS_TIME)
      {
        *result = BROWSER_FILE_LONGPRESS;
        break;
      }
    }
    strncpy(output->fname, file_list[r - dir_count]->filename, sizeof(output->fname));
    output->fname[sizeof(output->fname) - 1] = 0;
    output->fsize = file_list[r - dir_count]->fsize;
    output->fattrib = file_list[r - dir_count]->fattrib;
  }
  if (!is_selected)
    *result = BROWSER_BACK;

  browser_free();

  return FR_OK;
}

// state file tree browser
// directory - start directory and new directory path output
// dir_max_len - size of "directory"
// fno - start file and selected file output
// returns output with selection result
BROWSER_RESULT browser_tree(char *directory, int dir_max_len, FILINFO *fno)
{
  BROWSER_RESULT br;
  FRESULT fr;
  int i;

  while (1)
  {
    fr = browser(directory, fno, &br, fno->fname);
    if (fr == FR_NO_PATH) // directory not exists (anymore?)
    {
      // repeat from root
      directory[0] = 0;
      fr = browser(directory, fno, &br, fno->fname);
    }
    show_error_screen_fr(fr, 1);
    switch (br)
    {
    case BROWSER_BACK:
      // "back" button pressed
      if (!*directory)
      {
        // we are at root already, return
        return BROWSER_BACK;
      }
      for (i = strlen(directory) - 2; i >= 0; i--)
      {
        // return to previous directory
        if (i <= 0)
        {
          // root
          strncpy(fno->fname, directory + (*directory == '\\' ? 1 : 0), sizeof(fno->fname));
          fno->fname[sizeof(fno->fname) - 1] = 0;
          directory[0] = 0;
        }
        if (directory[i] == '\\')
        {
          // extract parent directory and current directory names
          directory[i] = 0;
          strncpy(fno->fname, &directory[i + 1], sizeof(fno->fname));
          fno->fname[sizeof(fno->fname) - 1] = 0;
          if (i < dir_max_len) directory[i + 1] = 0;
          break;
        }
      }
      break;
    case BROWSER_DIRECTORY:
      // directory selected
      strncat(directory, "\\", dir_max_len);
      strncat(directory, fno->fname, dir_max_len);
      directory[dir_max_len - 1] = 0;
      fno->fname[0] = 0;
      break;
    case BROWSER_FILE:
      // file selected
    case BROWSER_FILE_LONGPRESS:
      // file selected using button longpress
      return br;
    }
  }
}

// free allocated memory
void browser_free()
{
  int i;
  for (i = 0; i < dir_count; i++)
  {
    free(dir_list[i]->filename);
    free(dir_list[i]);
  }for (i = 0; i < file_count; i++)
  {
    free(file_list[i]->filename);
    free(file_list[i]);
  }
  if (dir_list)
    free(dir_list);
  if (file_list)
    free(file_list);
  file_list = 0;
  dir_list = 0;
  dir_count = 0;
  file_count = 0;
}
