#ifndef INC_BLUPDATER_H_
#define INC_BLUPDATER_H_

#define BOOTLOADER_FILE "bootloader.bin"
#define BOOTLOADER_MD5_FILE "bootloader.bin.md5"
#define BOOTLOADER_ADDRESS FLASH_BASE
#define BOOTLOADER_MAX_SIZE (128 * 1024)

void update_bootloader();

#endif /* INC_BLUPDATER_H_ */
