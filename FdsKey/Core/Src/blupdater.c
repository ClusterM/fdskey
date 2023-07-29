#include "main.h"
#include "blupdater.h"
#include "oled.h"
#include "app_fatfs.h"
#include "splash.h"
#include "confirm.h"
#include "fdsemu.h"
#include "md5.h"

void update_bootloader()
{
  FRESULT fr;
  FIL fp;
  FILINFO fno, fno_bl;
  uint64_t buffer[FLASH_PAGE_SIZE / sizeof(uint64_t)];
  UINT br;
  int pos, i;
  HAL_StatusTypeDef r;
  FLASH_EraseInitTypeDef erase_init_struct;
  uint32_t sector_error = 0;
  uint8_t* bootloader_data;
  MD5Context md5_ctx;
  char md5_buffer[5];
  unsigned int md5_byte;

  show_message("Please wait...", 0);

  fr = f_stat(BOOTLOADER_FILE, &fno);
  if (fr == FR_NO_FILE)
  {
      show_error_screen(BOOTLOADER_FILE " not found", 0);
      return;
  } else if (fr != FR_OK)
  {
    show_error_screen_fr(fr, 0);
    return;
  }
  if (fno.fsize > BOOTLOADER_MAX_SIZE)
  {
    show_error_screen("File is too big", 1);
    return;
  }
  fr = f_stat(BOOTLOADER_FILE, &fno_bl);
  if (fr == FR_NO_FILE)
  {
      show_error_screen("MD5 file not found", 0);
      return;
  } else if (fr != FR_OK)
  {
    show_error_screen_fr(fr, 0);
    return;
  }

  fr = f_open(&fp, BOOTLOADER_FILE, FA_READ);
  if (fr != FR_OK)
  {
    show_error_screen_fr(fr, 0);
    return;
  }

  bootloader_data = malloc(fno.fsize + FLASH_PAGE_SIZE);
  if (!bootloader_data)
  {
    f_close(&fp);
    show_error_screen_fr(FDSR_OUT_OF_MEMORY, 0);
    return;
  }

  pos = 0;
  md5Init(&md5_ctx);
  do
  {
    fr = f_read(&fp, bootloader_data + pos, FLASH_PAGE_SIZE, &br);
    if (fr != FR_OK)
    {
      free(bootloader_data);
      f_close(&fp);
      show_error_screen_fr(fr, 0);
      return;
    }
    md5Update(&md5_ctx, bootloader_data + pos, br);
    pos += br;
  } while (br);
  f_close(&fp);
  md5Finalize(&md5_ctx);

  // read and check MD5
  fr = f_open(&fp, BOOTLOADER_MD5_FILE, FA_READ);
  if (fr != FR_OK)
  {
    free(bootloader_data);
    show_error_screen_fr(fr, 0);
    return;
  }
  for (i = 0; i < sizeof(md5_ctx.digest); i++)
  {
    fr = f_read(&fp, md5_buffer + 2, 2, &br);
    if (fr != FR_OK)
    {
      free(bootloader_data);
      f_close(&fp);
      show_error_screen_fr(fr, 0);
      return;
    }
    md5_buffer[0] = '0';
    md5_buffer[1] = 'x';
    md5_buffer[4] = 0;
    sscanf(md5_buffer, "%x", &md5_byte);
    if ((br != 2) || (md5_ctx.digest[i] != md5_byte))
    {
      free(bootloader_data);
      f_close(&fp);
      show_error_screen("Invalid MD5 checksum", 0);
      return;
    }
  }
  f_close(&fp);

  //show_updating_screen();
  show_message("Updating...\nKEEP POWER ON!", 0);

  r = HAL_FLASH_Unlock();
  if (r != HAL_OK)
  {
    free(bootloader_data);
    show_error_screen("Flash unlock error", 0);
    return;
  }

  pos = 0;
  do
  {
    memcpy(buffer, bootloader_data + pos, FLASH_PAGE_SIZE);
    erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init_struct.Banks = ((BOOTLOADER_ADDRESS - 0x08000000 + pos) / FLASH_BANK_SIZE == 0) ? FLASH_BANK_1 : FLASH_BANK_2;
    erase_init_struct.Page = ((BOOTLOADER_ADDRESS - 0x08000000 + pos) / FLASH_PAGE_SIZE) % FLASH_PAGE_NB;
    erase_init_struct.NbPages = 1;
    r = HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
    if (r != HAL_OK)
    {
      free(bootloader_data);
      f_close(&fp);
      HAL_FLASH_Lock();
      show_error_screen("Sector erase error", 0);
      show_error_screen("Bricked?", 1);
    }
    for (i = 0; i < FLASH_PAGE_SIZE; i += sizeof(uint64_t))
    {
      r = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, BOOTLOADER_ADDRESS + pos, buffer[i / sizeof(uint64_t)]);
      if (r != HAL_OK)
      {
        free(bootloader_data);
        f_close(&fp);
        HAL_FLASH_Lock();
        show_error_screen("Flash writing error", 0);
        show_error_screen("Bricked?", 1);
      }
      pos += sizeof(uint64_t);
    }
  } while (pos < fno.fsize);

  HAL_FLASH_Lock();
  free(bootloader_data);
  show_message("Bootloader updated", 0);
  HAL_Delay(1500);

  if (confirm("Delete " BOOTLOADER_FILE "?"))
  {
    // clear screen
    oled_draw_rectangle(0, oled_get_line() + OLED_HEIGHT, OLED_WIDTH - 1, oled_get_line() + OLED_HEIGHT + OLED_HEIGHT - 1, 1, 0);
    oled_update_invisible();
    oled_switch_to_invisible();
    // delete file
    fr = f_unlink(BOOTLOADER_FILE);
      show_error_screen_fr(fr, 0);
    fr = f_unlink(BOOTLOADER_MD5_FILE);
      show_error_screen_fr(fr, 0);
  }

  // unmount
  f_mount(0, "", 1);

  show_message("Done!", 0);
  HAL_Delay(1500);

  // enable watchdog
  // simple way to reset the device
  IWDG->KR = 0xCCCC;
  IWDG->KR = 0x5555;
  IWDG->PR = 0;
  IWDG->RLR = 1;
  while (1);
}
