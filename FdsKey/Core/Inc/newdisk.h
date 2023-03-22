#ifndef INC_NEWDISK_H_
#define INC_NEWDISK_H_

#include "app_fatfs.h"

#define NEW_DISK_FILENAME_LENGTH 10
#define NEW_DISK_FILENAME_START_CHAR 32
#define NEW_DISK_FILENAME_END_CHAR 90

FRESULT new_disk();

#endif /* INC_NEWDISK_H_ */
