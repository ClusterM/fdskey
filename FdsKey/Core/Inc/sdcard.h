#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "main.h"

#define SD_SPI_PORT      hspi3
//#define SD_CS_Pin        GPIO_PIN_5
//#define SD_CS_GPIO_Port  GPIOA

#define SD_TIMEOUT       100 // milliseconds
#define SD_INIT_TRIES    512

#define SD_R1_IDLE (1 << 0)
#define SD_R1_ERASE_CLEARED (1 << 1)
#define SD_R1_ILLEGAL_COMMAND (1 << 2)
#define SD_R1_CRC_ERROR (1 << 3)
#define SD_R1_ERASE_COMMANDS_ERROR (1 << 4)
#define SD_R1_MISALIGNED_ADDRESS (1 << 5)
#define SD_R1_ARGUMENT_OUTSIDE_OF_RANGE (1 << 6)

#define SD_DATA_TOKEN 0xFE
#define SD_SEND_MULTIPLE_DATA_TOKEN 0xFC
#define SD_STOP_DATA_TOKEN 0xFD

extern SPI_HandleTypeDef SD_SPI_PORT;

HAL_StatusTypeDef SD_init();
uint64_t SD_capacity();
HAL_StatusTypeDef SD_read_single_block(uint32_t blockNum, uint8_t* buff); // sizeof(buff) == 512!
HAL_StatusTypeDef SD_write_single_block(uint32_t blockNum, const uint8_t* buff); // sizeof(buff) == 512!

// Read Multiple Blocks
HAL_StatusTypeDef SD_read_begin(uint32_t blockNum);
HAL_StatusTypeDef SD_read_data(uint8_t* buff); // sizeof(buff) == 512!
HAL_StatusTypeDef SD_read_end();

// Write Multiple Blocks
HAL_StatusTypeDef SD_write_begin(uint32_t blockNum);
HAL_StatusTypeDef SD_write_data(const uint8_t* buff); // sizeof(buff) == 512!
HAL_StatusTypeDef SD_write_end();

// TODO: read lock flag? CMD13, SEND_STATUS

#endif // __SDCARD_H__
