/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sdcard.h"
#include "splash.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  SD_RESULT r = SD_init_try_speed();
  if (r != SD_RES_OK)
    show_error_screen_sd(r, 1);
  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
  SD_RESULT r;
  if (count == 1)
  {
    r = SD_read_single_block(sector, buff);
    if (r != SD_RES_OK)
      show_error_screen_sd(r, 1);
  } else {
    r = SD_read_begin(sector);
    if (r != SD_RES_OK)
      show_error_screen_sd(r, 1);
    while (count) {
      r = SD_read_data(buff);
      if (r != SD_RES_OK)
        show_error_screen_sd(r, 1);
      buff += FF_MIN_SS;
      count--;
    }
    r = SD_read_end();
    if (r != SD_RES_OK)
      show_error_screen_sd(r, 1);
  }
  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
  SD_RESULT r;
  if (count == 1)
  {
    r = SD_write_single_block(sector, buff);
    if (r != SD_RES_OK)
      show_error_screen_sd(r, 1);
  } else {
    r = SD_write_begin(sector);
    if (r != SD_RES_OK)
      show_error_screen_sd(r, 1);
    while (count) {
      r = SD_write_data(buff);
      if (r != SD_RES_OK)
        show_error_screen_sd(r, 1);
      buff += FF_MIN_SS;
      count--;
    }
    r = SD_write_end();
    if (r != SD_RES_OK)
      show_error_screen_sd(r, 1);
  }
  return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  switch (cmd)
  {
  case GET_SECTOR_COUNT:
    *((DWORD*)buff) = SD_read_capacity() / FF_MIN_SS;
    return RES_OK;
  case CTRL_SYNC:
    return RES_OK;
  }
  return RES_ERROR;
}

DWORD get_fattime (void) /* Get current time */
{
  return 0;
}
