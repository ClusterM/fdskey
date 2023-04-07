/* vim: set ai et ts=4 sw=4: */

#include "sdcard.h"

static uint64_t card_capacity;
static uint8_t sd_version;
static uint8_t sd_high_capacity;

static void SD_Select()
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void SD_Unselect()
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

static HAL_StatusTypeDef SPI_TransmitReceive(uint8_t* tx, uint8_t* rx, size_t buff_size)
{
//  uint8_t i, o, bit;
  return HAL_SPI_TransmitReceive(&SD_SPI_PORT, tx, rx, buff_size, SD_TIMEOUT);
//  while (buff_size)
//  {
//    i = *tx;
//    o = 0;
//    for (bit = 0; bit < 8; bit++)
//    {
//      HAL_GPIO_WritePin(SD_MOSI_GPIO_Port, SD_MOSI_Pin, (i >> (7 - bit)) & 1);
//      HAL_GPIO_WritePin(SD_CLK_GPIO_Port, SD_CLK_Pin, GPIO_PIN_SET);
//      o <<= 1;
//      o |= HAL_GPIO_ReadPin(SD_MISO_GPIO_Port, SD_MISO_Pin) & 1;
//      HAL_GPIO_WritePin(SD_CLK_GPIO_Port, SD_CLK_Pin, GPIO_PIN_RESET);
//    }
//    *rx = o;
//    tx++;
//    rx++;
//    buff_size--;
//  }
//  return HAL_OK;
}

static HAL_StatusTypeDef SPI_Transmit(uint8_t* tx, size_t buff_size)
{
//  uint8_t i, bit;
  return HAL_SPI_Transmit(&SD_SPI_PORT, tx, buff_size, SD_TIMEOUT);
//  while (buff_size)
//  {
//    i = *tx;
//    for (bit = 0; bit < 8; bit++)
//    {
//      HAL_GPIO_WritePin(SD_MOSI_GPIO_Port, SD_MOSI_Pin, (i >> (7 - bit)) & 1);
//      HAL_GPIO_WritePin(SD_CLK_GPIO_Port, SD_CLK_Pin, GPIO_PIN_SET);
//      HAL_GPIO_WritePin(SD_CLK_GPIO_Port, SD_CLK_Pin, GPIO_PIN_RESET);
//    }
//    tx++;
//    buff_size--;
//  }
//  return HAL_OK;
}

static HAL_StatusTypeDef SD_read_bytes(uint8_t *buff, size_t buff_size)
{
  HAL_StatusTypeDef r;
  // make sure FF is transmitted during receive
  uint8_t tx = 0xFF;
  while (buff_size > 0)
  {
    r = SPI_TransmitReceive(&tx, buff, 1);
    if (r != HAL_OK) return r;
    buff++;
    buff_size--;
  }

  return 0;
}

/*
 R1: 0abcdefg
 ||||||`- 1th bit (g): card is in idle state
 |||||`-- 2th bit (f): erase sequence cleared
 ||||`--- 3th bit (e): illegal command detected
 |||`---- 4th bit (d): crc check error
 ||`----- 5th bit (c): error in the sequence of erase commands
 |`------ 6th bit (b): misaligned address used in command
 `------- 7th bit (a): command argument outside allowed range
 (8th bit is always zero)
 */

static HAL_StatusTypeDef SD_read_r1(uint8_t *r1)
{
  HAL_StatusTypeDef r;
  uint8_t tx = 0xFF;
  uint32_t start_time = HAL_GetTick();
  do
  {
    r = SPI_TransmitReceive(&tx, r1, sizeof(uint8_t));
    if (r != HAL_OK)
      return r;
    if (HAL_GetTick() >= start_time + SD_TIMEOUT)
      return HAL_TIMEOUT;
  } while (*r1 & (1 << 7));
  return HAL_OK;
}

static HAL_StatusTypeDef SD_read_rx(uint8_t *rx, uint8_t x)
{
  HAL_StatusTypeDef r;
  int i;
  uint8_t tx = 0xFF;
  uint32_t start_time = HAL_GetTick();
  r = SD_read_r1(rx);
  if (r != HAL_OK)
    return r;
  rx++;
  for (i = 0; i < x; i++)
  {
    r = SPI_TransmitReceive(&tx, rx, sizeof(uint8_t));
    if (r != HAL_OK)
      return r;
    if (HAL_GetTick() >= start_time + SD_TIMEOUT)
      return HAL_TIMEOUT;
    rx++;
  }
  return HAL_OK;
}

static HAL_StatusTypeDef SD_read_r3(uint8_t *r3)
{
  return SD_read_rx(r3, 4);
}

static HAL_StatusTypeDef SD_read_r7(uint8_t *r7)
{
  return SD_read_rx(r7, 4);
}

static HAL_StatusTypeDef SD_wait_not_busy()
{
  HAL_StatusTypeDef r;
  uint32_t start_time = HAL_GetTick();
  uint8_t busy;
  do
  {
    r = SD_read_bytes(&busy, sizeof(busy));
    if (r != HAL_OK)
      return r;
    if (HAL_GetTick() >= start_time + SD_TIMEOUT)
      return HAL_TIMEOUT;
  } while (busy != 0xFF);
  return HAL_OK;
}

static HAL_StatusTypeDef SD_send_cmd(uint8_t command, uint32_t arg, uint8_t crc)
{
  HAL_StatusTypeDef r;
  uint8_t cmd[] = { 0x40 | command, (arg >> 24) & 0xFF, (arg >> 16) & 0xFF, (arg >> 8) & 0xFF, arg & 0xFF, (crc << 1) | 1 };
  r = SD_wait_not_busy();
  if (r != HAL_OK)
    return r;
  return SPI_Transmit((uint8_t*) cmd, sizeof(cmd));
}

static HAL_StatusTypeDef SD_send_acmd(uint8_t command, uint32_t arg)
{
  HAL_StatusTypeDef r;
  uint8_t r1;
  r = SD_send_cmd(55, 0, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != SD_R1_IDLE)
    return HAL_ERROR;
  return SD_send_cmd(command, arg, 0xFF);
}

static HAL_StatusTypeDef SD_wait_data_token()
{
  HAL_StatusTypeDef r;
  uint8_t fb;
  // make sure FF is transmitted during receive
  uint8_t tx = 0xFF;
  while (1)
  {
    r = SPI_TransmitReceive(&tx, &fb, sizeof(fb));
    if (r != HAL_OK)
      return r;
    if (fb == SD_DATA_TOKEN)
      break;
    if (fb != 0xFF)
      return HAL_ERROR;
  }
  return HAL_OK;
}

HAL_StatusTypeDef SD_init()
{
  HAL_StatusTypeDef r;
  uint8_t r1;
  uint8_t r3[5];
  uint8_t r7[5];
  SD_CSD csd;
  int i;

  SD_Unselect();

  // 80 clock pulses to enter SPI mode
  uint8_t high = 0xFF;
  for (int i = 0; i < 10; i++)
  {
    r = SPI_Transmit(&high, sizeof(high));
    if (r != HAL_OK)
      return r;
  }

  SD_Select();

  // CMD0 - reset card
  for (i = 0; ; i++)
  {
    r = SD_send_cmd(0, 0x00000000, 0x4A);
    if (r != HAL_OK)
      return r;
    r = SD_read_r1(&r1);
    if (r != HAL_OK)
      return r;
    if (r1 == SD_R1_IDLE)
      break;
    if (i == SD_INIT_TRIES - 1)
      return HAL_ERROR;
  }

  // CMD8 - send interface condition
  r = SD_send_cmd(8, 0x000001AA, 0x43); // request 2.7-3.63v
  if (r != HAL_OK)
    return r;
  r = SD_read_r7(r7);
  if (r != HAL_OK)
    return r;
  if (r7[0] & SD_R1_ILLEGAL_COMMAND)
  {
    // command not supported - old SD card version
    sd_version = 1;
    sd_high_capacity = 0;
  } else
  {
    // new version
    if (r7[0] != 0x01)
      return HAL_ERROR;
    // check that voltage accepted and echo byte
    if (((r7[3] & 0x01) != 1) || (r7[4] != 0xAA))
      return HAL_ERROR;
    sd_version = 2;
  }

  // CMD58 - read OCR register
  r = SD_send_cmd(58, 0x00000000, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r3(r3);
  if (r != HAL_OK)
    return r;
  if (r3[0] & 0b11111110)
    return HAL_ERROR;
  // another voltage check
  if (!(r3[2] & (0b00110000))) // 3.2-3.3V or 3.3-3.4V
    return HAL_ERROR;

  for (i = 0; ; i++)
  {
    // ACMD41 - send operating condition
    r = SD_send_acmd(41, 0x40000000);
    if (r != HAL_OK)
      return r;
    r = SD_read_r1(&r1);
    if (r != HAL_OK)
      return r;
    if (r1 == 0x00)
      break; // initialization finished
    if (r1 != SD_R1_IDLE)
      return HAL_ERROR;
    if (i == SD_INIT_TRIES - 1)
      return HAL_ERROR;
  }

  if (sd_version == 2)
  {
    // CMD58 - read OCR register, again
    r = SD_send_cmd(58, 0x00000000, 0xFF);
    if (r != HAL_OK)
      return r;
    r = SD_read_r3(r3);
    if (r != HAL_OK)
      return r;
    if (r3[0] & 0b11111110)
      return HAL_ERROR;
    // check if power up ok and high capacity
    if (!(r3[1] & 0x80))
      return HAL_ERROR;
    sd_high_capacity = (r3[1] & 0x40) >> 6;
  }

  r = SD_read_csd(&csd);
  if (r != HAL_OK)
    return r;
  if (sd_high_capacity)
    card_capacity = (csd.version.v2.DeviceSize + 1) * SD_BLOCK_LENGTH * 1024;
  else
    card_capacity = (csd.version.v1.DeviceSize + 1) * (1UL << (csd.version.v1.DeviceSizeMul + 2)) * (1UL << csd.RdBlockLen);

  // set block length
  r = SD_send_cmd(16, SD_BLOCK_LENGTH, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != 0x00)
    return HAL_ERROR;

  SD_Unselect();
  return HAL_OK;
}

// reduce SPI speed until SD card init is ok
HAL_StatusTypeDef SD_init_try_speed()
{
  HAL_StatusTypeDef r;
  SD_SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  HAL_SPI_Init(&SD_SPI_PORT);
  r = SD_init();
  if (r == HAL_OK)
    return r;
  SD_SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  HAL_SPI_Init(&SD_SPI_PORT);
  r = SD_init();
  if (r == HAL_OK)
    return r;
  SD_SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  HAL_SPI_Init(&SD_SPI_PORT);
  r = SD_init();
  if (r == HAL_OK)
    return r;
  SD_SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  HAL_SPI_Init(&SD_SPI_PORT);
  return SD_init();
}

HAL_StatusTypeDef SD_read_csd(SD_CSD* csd)
{
  HAL_StatusTypeDef r;
  uint8_t r1;
  uint8_t csd_data[16];
  uint8_t crc[2];

  // CMD9 - read CSD register and SD card capacity
  r = SD_send_cmd(9, 0x00000000, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != 0x00)
    return HAL_ERROR;
  r = SD_wait_data_token();
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(csd_data, sizeof(csd_data));
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(crc, sizeof(crc));
  if (r != HAL_OK)
    return r;

  csd->CSDStruct = (csd_data[0] & 0xC0) >> 6;
  csd->Reserved1 = csd_data[0] & 0x3F;
  csd->TAAC = csd_data[1];
  csd->NSAC = csd_data[2];
  csd->MaxBusClkFrec = csd_data[3];
  csd->CardComdClasses = (csd_data[4] << 4) | ((csd_data[5] & 0xF0) >> 4);
  csd->RdBlockLen = csd_data[5] & 0x0F;
  csd->PartBlockRead = (csd_data[6] & 0x80) >> 7;
  csd->WrBlockMisalign = (csd_data[6] & 0x40) >> 6;
  csd->RdBlockMisalign = (csd_data[6] & 0x20) >> 5;
  csd->DSRImpl = (csd_data[6] & 0x10) >> 4;
  switch (csd->CSDStruct) {
  case 0:
    // CSD version 1
    csd->version.v1.Reserved1 = ((csd_data[6] & 0x0C) >> 2);
    csd->version.v1.DeviceSize = ((csd_data[6] & 0x03) << 10) | (csd_data[7] << 2) |
                                 ((csd_data[8] & 0xC0) >> 6);
    csd->version.v1.MaxRdCurrentVDDMin = (csd_data[8] & 0x38) >> 3;
    csd->version.v1.MaxRdCurrentVDDMax = (csd_data[8] & 0x07);
    csd->version.v1.MaxWrCurrentVDDMin = (csd_data[9] & 0xE0) >> 5;
    csd->version.v1.MaxWrCurrentVDDMax = (csd_data[9] & 0x1C) >> 2;
    csd->version.v1.DeviceSizeMul = ((csd_data[9] & 0x03) << 1) |
                                    ((csd_data[10] & 0x80) >> 7);
    break;
  case 1:
    // CSD version 2
    csd->version.v2.Reserved1 = ((csd_data[6] & 0x0F) << 2) |
                                ((csd_data[7] & 0xC0) >> 6);
    csd->version.v2.DeviceSize = ((csd_data[7] & 0x3F) << 16) | (csd_data[8] << 8) |
                                 csd_data[9];
    csd->version.v2.Reserved2 = ((csd_data[10] & 0x80) >> 8);
    break;
  default:
    return HAL_ERROR;
  }
  csd->EraseSingleBlockEnable = (csd_data[10] & 0x40) >> 6;
  csd->EraseSectorSize = ((csd_data[10] & 0x3F) << 1) | ((csd_data[11] & 0x80) >> 7);
  csd->WrProtectGrSize = (csd_data[11] & 0x7F);
  csd->WrProtectGrEnable = (csd_data[12] & 0x80) >> 7;
  csd->Reserved2 = (csd_data[12] & 0x60) >> 5;
  csd->WrSpeedFact = (csd_data[12] & 0x1C) >> 2;
  csd->MaxWrBlockLen = ((csd_data[12] & 0x03) << 2) | ((csd_data[13] & 0xC0) >> 6);
  csd->WriteBlockPartial = (csd_data[13] & 0x20) >> 5;
  csd->Reserved3 = (csd_data[13] & 0x1F);
  csd->FileFormatGrouop = (csd_data[14] & 0x80) >> 7;
  csd->CopyFlag = (csd_data[14] & 0x40) >> 6;
  csd->PermWrProtect = (csd_data[14] & 0x20) >> 5;
  csd->TempWrProtect = (csd_data[14] & 0x10) >> 4;
  csd->FileFormat = (csd_data[14] & 0x0C) >> 2;
  csd->Reserved4 = (csd_data[14] & 0x03);
  csd->crc = (csd_data[15] & 0xFE) >> 1;
  csd->Reserved5 = (csd_data[15] & 0x01);

  return HAL_OK;
}

uint64_t SD_get_capacity()
{
  return card_capacity;
}

HAL_StatusTypeDef SD_read_single_block(uint32_t blockNum, uint8_t *buff)
{
  HAL_StatusTypeDef r;
  uint8_t crc[2];
  uint8_t r1;

  SD_Select();

  if (!sd_high_capacity)
    blockNum *= SD_BLOCK_LENGTH;

  /* CMD17 (SEND_SINGLE_BLOCK) command */
  r = SD_send_cmd(17, blockNum, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != 0x00)
    return HAL_ERROR;
  r = SD_wait_data_token();
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(buff, SD_BLOCK_LENGTH);
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(crc, 2);
  if (r != HAL_OK)
    return r;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_write_single_block(uint32_t blockNum, const uint8_t *buff)
{
  HAL_StatusTypeDef r;
  uint8_t r1;

  SD_Select();

  if (!sd_high_capacity)
    blockNum *= SD_BLOCK_LENGTH;

  /* CMD24 (WRITE_BLOCK) command */
  r = SD_send_cmd(24, blockNum, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != 0x00)
    return HAL_ERROR;

  uint8_t dataToken = SD_DATA_TOKEN;
  uint8_t crc[2] = { 0xFF, 0xFF };
  r = SPI_Transmit(&dataToken, sizeof(dataToken));
  if (r != HAL_OK)
    return r;
  r = SPI_Transmit((uint8_t*) buff, SD_BLOCK_LENGTH);
  if (r != HAL_OK)
    return r;
  r = SPI_Transmit(crc, sizeof(crc));
  if (r != HAL_OK)
    return r;

  /*
   dataResp:
   xxx0abc1
   010 - Data accepted
   101 - Data rejected due to CRC error
   110 - Data rejected due to write error
   */
  uint8_t dataResp;
  r = SD_read_bytes(&dataResp, sizeof(dataResp));
  if (r != HAL_OK)
    return r;
  if ((dataResp & 0x1F) != 0x05)
    return HAL_ERROR;

  r = SD_wait_not_busy();
  if (r != HAL_OK)
    return r;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_read_begin(uint32_t blockNum)
{
  HAL_StatusTypeDef r;
  uint8_t r1;

  SD_Select();

  if (!sd_high_capacity)
    blockNum *= SD_BLOCK_LENGTH;

  /* CMD18 (READ_MULTIPLE_BLOCK) command */
  r = SD_send_cmd(18, blockNum, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != 0x00)
    return HAL_ERROR;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_read_data(uint8_t *buff)
{
  HAL_StatusTypeDef r;
  uint8_t crc[2];

  SD_Select();

  r = SD_wait_data_token();
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(buff, SD_BLOCK_LENGTH);
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(crc, 2);
  if (r != HAL_OK)
    return r;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_read_end()
{
  HAL_StatusTypeDef r;
  uint8_t r1;

  SD_Select();

  /* CMD12 (STOP_TRANSMISSION) */
  r = SD_send_cmd(12, 0x00000000, 0xFF);
  if (r != HAL_OK)
    return r;

  /*
   The received byte immediataly following CMD12 is a stuff byte, it should be
   discarded before receive the response of the CMD12
   */
  uint8_t stuffByte;
  r = SD_read_bytes(&stuffByte, sizeof(stuffByte));
  if (r != HAL_OK)
    return r;

  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_write_begin(uint32_t blockNum)
{
  HAL_StatusTypeDef r;
  uint8_t r1;

  SD_Select();

  if (!sd_high_capacity)
    blockNum *= SD_BLOCK_LENGTH;

  /* CMD25 (WRITE_MULTIPLE_BLOCK) command */
  r = SD_send_cmd(25, blockNum, 0xFF);
  if (r != HAL_OK)
    return r;
  r = SD_read_r1(&r1);
  if (r != HAL_OK)
    return r;
  if (r1 != 0x00)
    return HAL_ERROR;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_write_data(const uint8_t *buff)
{
  HAL_StatusTypeDef r;

  SD_Select();

  uint8_t dataToken = SD_SEND_MULTIPLE_DATA_TOKEN;
  uint8_t crc[2] = { 0xFF, 0xFF };
  r = SPI_Transmit(&dataToken, sizeof(dataToken));
  if (r != HAL_OK)
    return r;
  r = SPI_Transmit((uint8_t*) buff, SD_BLOCK_LENGTH);
  if (r != HAL_OK)
    return r;
  r = SPI_Transmit(crc, sizeof(crc));
  if (r != HAL_OK)
    return r;

  /*
   dataResp:
   xxx0abc1
   010 - Data accepted
   101 - Data rejected due to CRC error
   110 - Data rejected due to write error
   */
  uint8_t dataResp;
  r = SD_read_bytes(&dataResp, sizeof(dataResp));
  if (r != HAL_OK)
    return r;
  if ((dataResp & 0x1F) != 0x05)
    return HAL_ERROR;

  r = SD_wait_not_busy();
  if (r != HAL_OK)
    return r;

  SD_Unselect();
  return HAL_OK;
}

HAL_StatusTypeDef SD_write_end()
{
  HAL_StatusTypeDef r;

  SD_Select();

  uint8_t stopTran = SD_STOP_DATA_TOKEN; // stop transaction token for CMD25
  r = SPI_Transmit(&stopTran, sizeof(stopTran));
  if (r != HAL_OK)
    return r;

  // skip one byte before readyng "busy"
  // this is required by the spec and is necessary for some real SD-cards!
  uint8_t skipByte;
  r = SD_read_bytes(&skipByte, sizeof(skipByte));
  if (r != HAL_OK)
    return r;

  r = SD_wait_not_busy();
  if (r != HAL_OK)
    return r;

  SD_Unselect();
  return HAL_OK;
}
