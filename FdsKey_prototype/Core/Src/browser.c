#include <string.h>
#include "main.h"
#include "fatfs.h"

static char* dir_path = 0;
static char** dir_list = 0;
static char** file_list = 0;
static int dir_count = 0;
static int file_count = 0;

static void browser_free();

// Left source half is A[ iBegin:iMiddle-1].
// Right source half is A[iMiddle:iEnd-1   ].
// Result is B[ iBegin:iEnd-1   ].
void top_down_merge(char** A, int iBegin, int iMiddle, int iEnd, char** B)
{
  int i = iBegin, j = iMiddle, k;

  // While there are elements in the left or right runs...
  for (k = iBegin; k < iEnd; k++) {
    // If left run head exists and is <= existing right run head.
    if (i < iMiddle && (j >= iEnd || strcasecmp(A[i], A[j]) <= 0)) {
      B[k] = A[i];
      i = i + 1;
    } else {
      B[k] = A[j];
      j = j + 1;
    }
  }
}

// Split A[] into 2 runs, sort both runs into B[], merge both runs from B[] to A[]
// iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
void top_down_split_merge(char** B, int iBegin, int iEnd, char** A)
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

void top_down_merge_sort(char** A, int n)
{
  char* B[n];
  memcpy(B, A, sizeof(char*) * n);
  top_down_split_merge(B, 0, n, A); // sort data from B[] into A[]
}

FRESULT browser_load_dir(char *path, char *select)
{
  FRESULT fr;
  DIR dir;
  FILINFO fno;
  int mem_dir_count = 512;
  int mem_file_count = 512;
  dir_count = 0;
  file_count = 0;

  print("Loading files...");

  dir_path = malloc(strlen(path) + 1);
  strcpy(dir_path, path);

  dir_list = malloc(mem_dir_count * sizeof(char*));
  file_list = malloc(mem_file_count * sizeof(char*));

  // load files and directories names
  // TODO: handle out of memory error
  fr = f_opendir(&dir, path);
  if (fr != FR_OK) {
    browser_free();
    return fr;
  }
  while (1)
  {
    fr = f_readdir(&dir, &fno);
    if (fr != FR_OK || !fno.fname[0])
      break;
    if (!(fno.fattrib & AM_HID))
    {
      if (fno.fattrib & AM_DIR)
      {
        if (dir_count + 1 > mem_dir_count)
        {
          mem_dir_count *= 2;
          dir_list = realloc(dir_list, mem_dir_count * sizeof(char*));
        }
        dir_list[dir_count] = malloc(strlen(fno.fname) + 1);
        strcpy(dir_list[dir_count], fno.fname);
        dir_count++;
      } else {
        if (file_count + 1 > mem_file_count)
        {
          mem_file_count *= 2;
          file_list = realloc(file_list, mem_file_count * sizeof(char*));
        }
        file_list[file_count] = malloc(strlen(fno.fname) + 1);
        strcpy(file_list[file_count], fno.fname);
        file_count++;
      }
    }
  }
  dir_list = realloc(dir_list, dir_count * sizeof(char*));
  file_list = realloc(file_list, file_count * sizeof(char*));
  f_closedir(&dir);

  print("sorting...");

  // sort them
  top_down_merge_sort(dir_list, dir_count);
  top_down_merge_sort(file_list, file_count);

  for (int i = 0; i < file_count; i++)
  {
    print(file_list[i]);
    //HAL_Delay(500);
  }

  return FR_OK;
}

void browser_free()
{
  int i;
  if (dir_path) free(dir_path);
  for (i = 0; i < dir_count; i++)
    free(dir_list[i]);
  for (i = 0; i < file_count; i++)
    free(file_list[i]);
  if (dir_list)
    free(dir_list);
  if (file_list)
    free(file_list);
  dir_path = 0;
  file_list = 0;
  dir_list = 0;
  dir_count = 0;
  file_count = 0;
}
