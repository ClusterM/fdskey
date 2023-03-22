/* vim: set ai et ts=4 sw=4: */

#include "sdcard.h"

static uint64_t card_size;
static uint8_t sd_version;

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
  uint8_t csd[16];
  uint8_t crc[2];
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
  for (i = 0; i < 5; i++)
  {
    r = SD_send_cmd(0, 0x00000000, 0x4A);
    if (r != HAL_OK)
      return r;
    r = SD_read_r1(&r1);
    if (r != HAL_OK)
      return r;
    if (r1 == SD_R1_IDLE)
      break;
    else if (i == 5 - 1)
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

  while (1)
  {
    // ACMD41 - send operating condition
    // indicate we support high capacity cards
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
    if (!(r3[1] & 0xC0))
      return HAL_ERROR;
  }

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
  r = SD_read_bytes(csd, sizeof(csd));
  if (r != HAL_OK)
    return r;
  r = SD_read_bytes(crc, sizeof(crc));
  if (r != HAL_OK)
    return r;

  // first byte is VVxxxxxxxx where VV is csd.version
  if ((csd[0] & 0xC0) == 0x40)
  {
    // csd.version 2
    uint64_t c_size = ((csd[7] & 0b00111111) << 16) | (csd[8] << 8) | csd[9];
    card_size = (c_size + 1) * 512 * 1024;
  } else
  {
    // csd.version 1
    uint64_t c_size = ((uint16_t) ((csd[6] & 0x03) << 10) | (uint16_t) (csd[7] << 2) | (uint16_t) ((csd[8] & 0xC0) >> 6)) + 1;
    c_size = c_size << (((uint16_t) ((csd[9] & 0x03) << 1) | (uint16_t) ((csd[10] & 0x80) >> 7)) + 2);
    c_size = c_size << ((uint16_t) (csd[5] & 0x0F));
    card_size = c_size;
    // set block length
    r = SD_send_cmd(16, 512, 0xFF);
    if (r != HAL_OK)
      return r;
  }

  SD_Unselect();
  return HAL_OK;
}

uint64_t SD_capacity()
{
  return card_size;
}

HAL_StatusTypeDef SD_read_single_block(uint32_t blockNum, uint8_t *buff)
{
  HAL_StatusTypeDef r;
  uint8_t crc[2];
  uint8_t r1;

  SD_Select();

  if (sd_version == 1)
    blockNum *= 512;

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
  r = SD_read_bytes(buff, 512);
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

  if (sd_version == 1)
    blockNum *= 512;

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
  r = SPI_Transmit((uint8_t*) buff, 512);
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

  if (sd_version == 1)
    blockNum *= 512;

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
  r = SD_read_bytes(buff, 512);
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

  if (sd_version == 1)
    blockNum *= 512;

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
  r = SPI_Transmit((uint8_t*) buff, 512);
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
