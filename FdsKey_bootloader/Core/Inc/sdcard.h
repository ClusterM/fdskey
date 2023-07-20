#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "main.h"

#define SD_SPI_PORT      hspi3

#define SD_TIMEOUT       500 // milliseconds
#define SD_INIT_TRIES    512
#define SD_BLOCK_LENGTH  512

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

typedef struct {
    /* Header part */
    uint8_t CSDStruct : 2; /* CSD structure */
    uint8_t Reserved1 : 6; /* Reserved */
    uint8_t TAAC : 8; /* Data read access-time 1 */
    uint8_t NSAC : 8; /* Data read access-time 2 in CLK cycles */
    uint8_t MaxBusClkFrec : 8; /* Max. bus clock frequency */
    uint16_t CardComdClasses : 12; /* Card command classes */
    uint8_t RdBlockLen : 4; /* Max. read data block length */
    uint8_t PartBlockRead : 1; /* Partial blocks for read allowed */
    uint8_t WrBlockMisalign : 1; /* Write block misalignment */
    uint8_t RdBlockMisalign : 1; /* Read block misalignment */
    uint8_t DSRImpl : 1; /* DSR implemented */

    /* v1 or v2 struct */
    union csd_version {
        struct {
            uint8_t Reserved1 : 2; /* Reserved */
            uint16_t DeviceSize : 12; /* Device Size */
            uint8_t MaxRdCurrentVDDMin : 3; /* Max. read current @ VDD min */
            uint8_t MaxRdCurrentVDDMax : 3; /* Max. read current @ VDD max */
            uint8_t MaxWrCurrentVDDMin : 3; /* Max. write current @ VDD min */
            uint8_t MaxWrCurrentVDDMax : 3; /* Max. write current @ VDD max */
            uint8_t DeviceSizeMul : 3; /* Device size multiplier */
        } v1;
        struct {
            uint8_t Reserved1 : 6; /* Reserved */
            uint32_t DeviceSize : 22; /* Device Size */
            uint8_t Reserved2 : 1; /* Reserved */
        } v2;
    } version;

    uint8_t EraseSingleBlockEnable : 1; /* Erase single block enable */
    uint8_t EraseSectorSize : 7; /* Erase group size multiplier */
    uint8_t WrProtectGrSize : 7; /* Write protect group size */
    uint8_t WrProtectGrEnable : 1; /* Write protect group enable */
    uint8_t Reserved2 : 2; /* Reserved */
    uint8_t WrSpeedFact : 3; /* Write speed factor */
    uint8_t MaxWrBlockLen : 4; /* Max. write data block length */
    uint8_t WriteBlockPartial : 1; /* Partial blocks for write allowed */
    uint8_t Reserved3 : 5; /* Reserved */
    uint8_t FileFormatGrouop : 1; /* File format group */
    uint8_t CopyFlag : 1; /* Copy flag (OTP) */
    uint8_t PermWrProtect : 1; /* Permanent write protection */
    uint8_t TempWrProtect : 1; /* Temporary write protection */
    uint8_t FileFormat : 2; /* File Format */
    uint8_t Reserved4 : 2; /* Reserved */
    uint8_t crc : 7; /* Reserved */
    uint8_t Reserved5 : 1; /* always 1*/

} SD_CSD;

extern SPI_HandleTypeDef SD_SPI_PORT;

HAL_StatusTypeDef SD_init();
HAL_StatusTypeDef SD_init_try_speed();
HAL_StatusTypeDef SD_read_csd(SD_CSD* csd);
uint64_t SD_get_capacity();
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
