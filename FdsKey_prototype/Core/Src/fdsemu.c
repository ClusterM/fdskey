#include <string.h>
#include "main.h"
#include "fdsemu.h"
#include "fatfs.h"

static char fds_filename[FDS_MAX_FILE_PATH_LENGHT];
static uint8_t fds_side;
// loaded FDS data
static volatile uint8_t fds_raw_data[FDS_MAX_SIDE_SIZE];
static volatile uint8_t fds_read_buffer[FDS_READ_BUFFER_SIZE];
static volatile int fds_used_space = 0;
static volatile int fds_block_count = 0;
static volatile int fds_block_offsets[FDS_MAX_BLOCKS];
static volatile int fds_block_sizes[FDS_MAX_BLOCKS];

// state machine variables
static volatile FDS_STATE fds_state = FDS_OFF;
static volatile uint8_t fds_clock = 0;
static volatile int fds_current_byte = 0;
static volatile uint8_t fds_current_bit = 0;
static volatile uint8_t fds_last_value = 0;
static volatile uint8_t fds_write_carrier = 0;
static volatile uint32_t fds_not_ready_time = 0;
static volatile uint16_t fds_current_block_end = 0;
static volatile uint16_t fds_write_gap_skip = 0;
static volatile uint8_t fds_changed = 0;

static uint8_t fds_config_fast_rewind = 0;

static void fds_stop_reading();

// debug dumping
void fds_dump(char* filename)
{
  FIL fp;
  unsigned int l;

  f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE);
  f_write(&fp, (uint8_t*)fds_raw_data, sizeof(fds_raw_data), &l);
  f_close(&fp);
}

static uint16_t fds_crc(uint8_t* data, unsigned size) {
  //Do not include any existing checksum, not even the blank checksums 00 00 or FF FF.
  //The formula will automatically count 2 0x00 bytes without the programmer adding them manually.
  //Also, do not include the gap terminator (0x80) in the data.
  //If you wish to do so, change sum to 0x0000.
  uint16_t sum = 0x8000;
  uint16_t byte_index;
  uint8_t bit_index;
  uint8_t byte;

  for (byte_index = 0; byte_index < size + 2; byte_index++) {
    byte = byte_index < size ? data[byte_index] : 0x00;
    for (bit_index = 0; bit_index < 8; bit_index++) {
      uint8_t bit = (byte >> bit_index) & 1;
      uint8_t carry = sum & 1;
      sum = (sum >> 1) | (bit << 15);
      if (carry) sum ^= 0x8408;
    }
  }
  return sum;
}

// returns block size without gap but with crc
static uint16_t fds_get_block_size(int i, uint8_t include_gap, uint8_t include_crc)
{
  if (i == 0) return (include_gap ? FDS_FIRST_GAP_READ_BITS / 8 : 0) + 56 + (include_crc ? 2: 0); // disk info block
  if (i == 1) return (include_gap ? FDS_NEXT_GAPS_READ_BITS / 8 : 0) + 2 + (include_crc ? 2: 0);  // file amount block
  if (i % 2 == 0) return (include_gap ? FDS_NEXT_GAPS_READ_BITS / 8 : 0) + 16 + (include_crc ? 2: 0); // file header block
  // file data block - size stored in previous block
  return (include_gap ? FDS_NEXT_GAPS_READ_BITS / 8 : 0)
      + 1
      + (fds_raw_data[fds_block_offsets[i - 1] + FDS_NEXT_GAPS_READ_BITS / 8 + 0x0D]
      | (fds_raw_data[fds_block_offsets[i - 1] + FDS_NEXT_GAPS_READ_BITS / 8 + 0x0E] << 8))
      + (include_crc ? 2: 0);
}

static void fds_dma_fill_buffer(int pos, int length)
{
  // filling PWM DMA buffer with data
  uint8_t bit, value;
  switch (fds_state)
  {
  case FDS_READING:
  case FDS_READ_WAIT_READY:
    break;
  default:
    return;
  }
  while (length)
  {
    fds_clock ^= 1;
    bit = (fds_raw_data[fds_current_byte] >> (fds_current_bit / 2)) & 1;
    value = bit ^ fds_clock;
    if (value && !fds_last_value)
      fds_read_buffer[pos] = FDS_READ_IMPULSE_LENGTH - 1;
    else
      fds_read_buffer[pos] = 0;
    fds_last_value = value;
    fds_current_bit++;
    if (fds_current_bit > 15)
    {
      fds_current_bit = 0;
      fds_current_byte = (fds_current_byte + 1) % FDS_MAX_SIDE_SIZE;
      if (fds_current_byte == FDS_MAX_SIDE_SIZE - FDS_NOT_READY_BYTES && fds_state == FDS_READING)
      {
        // "end of head" meet
        HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_SET);
        fds_state = FDS_READ_WAIT_READY;
      }
      if (fds_current_byte == 0 && fds_state == FDS_READ_WAIT_READY)
      {
        // disk just rewinded and ready again
        HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_RESET);
        fds_state = FDS_READING;
      }
    }
    pos++;
    length--;
  }
}

static void fds_dma_half_callback(DMA_HandleTypeDef *hdma)
{
  fds_dma_fill_buffer(0, FDS_READ_BUFFER_SIZE / 2);
}

static void fds_dma_full_callback(DMA_HandleTypeDef *hdma)
{
  fds_dma_fill_buffer(FDS_READ_BUFFER_SIZE / 2, FDS_READ_BUFFER_SIZE / 2);
}

static void fds_start_reading()
{
  fds_current_bit = 0;
  FDS_READ_DMA.XferHalfCpltCallback = fds_dma_half_callback;
  FDS_READ_DMA.XferCpltCallback = fds_dma_full_callback;
  HAL_DMA_Start_IT(&FDS_READ_DMA,
      (uint32_t)fds_read_buffer,
      (uint32_t)&FDS_READ_PWM_TIMER.Instance->FDS_READ_PWM_TIMER_CHANNEL_REG,
      FDS_READ_BUFFER_SIZE);
  __HAL_TIM_ENABLE_DMA(&FDS_READ_PWM_TIMER, TIM_DMA_UPDATE);
  fds_dma_fill_buffer(0, FDS_READ_BUFFER_SIZE);
  FDS_READ_PWM_TIMER.Instance->ARR = FDS_READ_IMPULSE_LENGTH * 10 - 1;
  HAL_TIM_PWM_Start(&FDS_READ_PWM_TIMER, FDS_READ_PWM_TIMER_CHANNEL_CONST);
  fds_state = FDS_READING;
}

static void fds_stop_reading()
{
  HAL_DMA_Abort_IT(&FDS_READ_DMA);
  HAL_TIM_PWM_Stop(&FDS_READ_PWM_TIMER, FDS_READ_PWM_TIMER_CHANNEL_CONST);
}

static void fds_start_writing()
{
  int i;
  int gap_length;
  int fds_current_block = 0;

  // calculate current block
  for (i = 0; ; i++)
  {
    if (i >= fds_block_count)
    {
      // add new block
      if (i == 0)
        fds_block_offsets[i] = 0;
      else
        fds_block_offsets[i] = fds_block_offsets[i - 1] + fds_get_block_size(i - 1, 1, 1);
      fds_current_block = fds_block_count;
      fds_block_count++;
      break;
    }
    uint16_t block_size = fds_get_block_size(i, 1, 1);
    if (fds_current_byte < fds_block_offsets[i] + block_size)
    {
      fds_current_block = i;
      break;
    }
  }
  // update used space
  fds_used_space = fds_block_offsets[fds_block_count - 1] + fds_get_block_size(fds_block_count - 1, 1, 1);

  fds_current_byte = fds_block_offsets[fds_current_block];
  gap_length = fds_current_block == 0 ? FDS_FIRST_GAP_READ_BITS / 8 : FDS_NEXT_GAPS_READ_BITS / 8;
  fds_current_block_end = (fds_current_byte + gap_length + fds_get_block_size(fds_current_block, 0, 1)) % FDS_MAX_SIDE_SIZE;
  if (fds_current_block_end < fds_current_byte) {
    // seems like "end of head" meet
    HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_SET);
    return;
  }
  if (fds_current_block + 1 < fds_block_count && fds_current_block_end != fds_block_offsets[fds_current_block + 1])
  {
    // oops, next block overwrited or disaligned
    // trimming and erasing
    fds_block_count = fds_current_block + 1;
    memset((uint8_t*)fds_raw_data + fds_block_offsets[fds_current_block + 1], 0, FDS_SIDE_LENGTH - fds_block_offsets[fds_current_block + 1]);
  }
  // gap before data
  for (i = 0; i < gap_length - 1; i++)
    fds_raw_data[fds_current_byte++] = 0;
  fds_raw_data[fds_current_byte++] = 0x80; // gap terminator
  fds_write_gap_skip = 0;
  fds_changed = 1; // flag that ROM changed

   // start and reset timer
  HAL_TIM_Base_Start(&FDS_WRITE_TIMER);
  FDS_WRITE_TIMER.Instance->CNT = 0;

  // enable interrupt on write data
  HAL_GPIO_DeInit(FDS_WRITE_DATA_GPIO_Port, FDS_WRITE_DATA_Pin);
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = FDS_WRITE_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(FDS_WRITE_DATA_GPIO_Port, &GPIO_InitStruct);
  fds_state = FDS_WRITING_GAP;
}

static void fds_stop_writing()
{
  // disable interrupt on write data
  HAL_GPIO_DeInit(FDS_WRITE_DATA_GPIO_Port, FDS_WRITE_DATA_Pin);
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = FDS_WRITE_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(FDS_WRITE_DATA_GPIO_Port, &GPIO_InitStruct);
}

static void fds_stop()
{
  fds_stop_reading();
  fds_stop_writing();
  HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_SET);
  fds_state = FDS_IDLE;
}

FRESULT fds_get_sides_count(char *filename, uint8_t *count)
{
  FILINFO fno;
  FRESULT fr;

  fr = f_stat(filename, &fno);
  if (fr != FR_OK) return fr;
  if (fno.fsize % FDS_SIDE_LENGTH == FDS_HEADER_SIZE)
    fno.fsize -= FDS_HEADER_SIZE;
  if (fno.fsize % FDS_SIDE_LENGTH != 0)
    return FR_INVALID_OBJECT;
  *count = fno.fsize / FDS_SIDE_LENGTH;
  return fr;
}

void fds_reset()
{
  // reset state machine
  fds_clock = 0;
  fds_current_byte = 0;
  fds_current_bit = 0;
  fds_last_value = 0;
}

void fds_write_bit(uint8_t bit)
{
  if (fds_current_byte < fds_current_block_end)
    fds_raw_data[fds_current_byte] = (fds_raw_data[fds_current_byte] >> 1) | (bit << 7);
  fds_current_bit++;
  if (fds_current_bit > 7)
  {
    fds_current_bit = 0;
    fds_current_byte = (fds_current_byte + 1) % FDS_MAX_SIDE_SIZE;
  }
}

void fds_write_impulse()
{
  uint32_t time = FDS_WRITE_TIMER.Instance->CNT;
  //if (time < 512) return; // filter
  FDS_WRITE_TIMER.Instance->CNT = 0;
  switch (fds_state)
  {
    case FDS_WRITING_GAP:
    case FDS_WRITING:
      break;
    default:
      fds_stop_writing();
      return;
  }
  if (fds_state == FDS_WRITING_GAP)
  {
    if (fds_write_gap_skip < FDS_WRITE_GAP_SKIP_BITS)
      fds_write_gap_skip++;
    else if (time >= FDS_THRESHOLD_1)
    {
      // start '1' bit
      fds_write_carrier = 0;
      fds_current_bit = 0;
      fds_state = FDS_WRITING;
    }
  } else if (fds_state == FDS_WRITING)
  {
    uint8_t l = fds_write_carrier;
    if (time < FDS_THRESHOLD_1)
      l |= 2;
    else if (time < FDS_THRESHOLD_2)
      l |= 3;
    else
      l |= 4;
    switch (l)
    {
    case 0x82:
      fds_write_bit(0);
      fds_write_carrier = 0x80;
      break;
    case 0x83:
      fds_write_bit(1);
      fds_write_carrier = 0;
      break;
    case 0x84:
      // invalid state
      break;
    case 0x02:
      fds_write_bit(1);
      fds_write_carrier = 0;
      break;
    case 0x03:
      fds_write_bit(0);
      fds_write_bit(0);
      fds_write_carrier = 0x80;
      break;
    case 0x04:
      fds_write_bit(0);
      fds_write_bit(1);
      fds_write_carrier = 0;
      break;
    }
  }
}

void fds_check_pins()
{
  if (HAL_GPIO_ReadPin(FDS_SCAN_MEDIA_GPIO_Port, FDS_SCAN_MEDIA_Pin))
  {
    // motor stop
    HAL_GPIO_WritePin(FDS_MOTOR_ON_GPIO_Port, FDS_MOTOR_ON_Pin, GPIO_PIN_RESET); // do i really need this?
    if (fds_state != FDS_IDLE)
      fds_stop();
  } else {
    HAL_GPIO_WritePin(FDS_MOTOR_ON_GPIO_Port, FDS_MOTOR_ON_Pin, GPIO_PIN_SET);
    if (HAL_GPIO_ReadPin(FDS_WRITE_GPIO_Port, FDS_WRITE_Pin))
    {
      // reading
      switch (fds_state)
      {
      case FDS_IDLE:
        fds_start_reading();
        if (fds_config_fast_rewind || fds_current_byte == 0)
        {
          fds_reset();
          fds_not_ready_time = HAL_GetTick();
          fds_state = FDS_READ_WAIT_READY_TIMER;
        } else {
          fds_state = FDS_READ_WAIT_READY;
        }
        break;
      case FDS_WRITING:
      case FDS_WRITING_GAP:
        fds_stop_writing();
        fds_start_reading();
        break;
      default:
        break;
      }
    } else {
      // writing
      switch (fds_state)
      {
      case FDS_IDLE:
      case FDS_READING:
      case FDS_READ_WAIT_READY:
      case FDS_READ_WAIT_READY_TIMER:
        fds_stop_reading();
        fds_start_writing();
        break;
      default:
        break;
      }
    }
  }
}

void fds_tick_100ms() // call every ~100ms, low priority task
{
  if (fds_state == FDS_OFF) return;
  if (fds_state == FDS_READ_WAIT_READY_TIMER)
  {
    if (fds_not_ready_time + FDS_NOT_READY_TIME < HAL_GetTick())
    {
      fds_state = FDS_READING;
      HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_RESET);
    }
  }
}

FRESULT fds_load_side(char *filename, uint8_t side)
{
  int i;
  FILINFO fno;
  FIL fp;
  FRESULT fr;
  int gap_length;
  int block_size;
  uint8_t block_type;
  int next_file_size = 0;
  UINT br;
  uint16_t crc;

  fds_close();
  fds_reset();

  // not ready yet
  HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_SET);
  // but media is inserted
  HAL_GPIO_WritePin(FDS_MEDIA_SET_GPIO_Port, FDS_MEDIA_SET_Pin, GPIO_PIN_RESET);
  // writable maybe
  HAL_GPIO_WritePin(FDS_WRITABLE_MEDIA_GPIO_Port, FDS_WRITABLE_MEDIA_Pin, GPIO_PIN_RESET);

  strncpy(fds_filename, filename, sizeof(fds_filename));
  fds_side = side;

  fr = f_stat(filename, &fno);
  if (fr != FR_OK) return fr;
  fr = f_open(&fp, filename, FA_READ);
  if (fr != FR_OK) return fr;
  if (fno.fsize % FDS_SIDE_LENGTH != 0 && fno.fsize % FDS_SIDE_LENGTH != 16)
    return FR_INVALID_OBJECT;
  fr = f_lseek(&fp, ((fno.fsize % FDS_SIDE_LENGTH == FDS_HEADER_SIZE) ? FDS_HEADER_SIZE : 0) + side * FDS_SIDE_LENGTH);
  if (fr != FR_OK) return fr;

  while(1)
  {
    fds_block_offsets[fds_block_count] = fds_used_space;
    gap_length = fds_block_count == 0 ? FDS_FIRST_GAP_READ_BITS / 8 : FDS_NEXT_GAPS_READ_BITS / 8;
    // gap before data
    for (i = 0; i < gap_length - 1; i++)
    {
     fds_raw_data[fds_used_space++] = 0;
     if (fds_used_space - 1 >= sizeof(fds_raw_data))
     {
       fds_used_space -= i;
       break;
     }
    }
    fds_raw_data[fds_used_space++] = 0x80; // gap terminator

    if (fds_block_count == 0)
    {
      // disk info block
      block_type = 1;
      block_size = 56;
    }
    else if (fds_block_count == 1)
    {
      // file amount block
      block_type = 2;
      block_size = 2;
    }
    else if (fds_block_count % 2 == 0)
    {
      // file header block
      block_type = 3;
      block_size = 16;
    }
    else {
      // file data block
      block_type = 4;
      block_size = next_file_size + 1;
    }

    if (fds_used_space + block_size + 2 /*CRC*/ > FDS_SIDE_LENGTH)
    {
      fds_used_space -= gap_length; // rollback last gap
      break;
    }

    // reading
    fr = f_read(&fp, (uint8_t*)fds_raw_data + fds_used_space, block_size, &br);
    if (fr != FR_OK)
    {
     // SD card error?
     f_close(&fp);
     fds_close();
     return fr;
    }
    if (br != block_size) {
     // end of file?
     fds_used_space -= gap_length; // rollback last gap
     break;
    }
    if (fds_raw_data[fds_used_space] != block_type)
    {
     // invalid block?
     fds_used_space -= gap_length; // rollback last gap
     break;
    }
    if (block_type == 3) // file header block - parse file size
     next_file_size = fds_raw_data[fds_used_space + 0x0D] | (fds_raw_data[fds_used_space + 0x0E] << 8);
    crc = fds_crc((uint8_t*)fds_raw_data + fds_used_space, block_size);
    fds_block_sizes[fds_block_count] = block_size + 2; // block + crc
    fds_used_space += block_size;
    fds_raw_data[fds_used_space++] = crc & 0xFF;
    fds_raw_data[fds_used_space++] = (crc >> 8) & 0xFF;
     fds_block_count++;
  }
  f_close(&fp);

  if (!HAL_GPIO_ReadPin(FDS_SCAN_MEDIA_GPIO_Port, FDS_SCAN_MEDIA_Pin))
  {
    fds_start_reading();
    HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_RESET);
  } else {
    fds_state = FDS_IDLE;
  }
 
  return FR_OK;
}

void fds_close()
{
  fds_stop();
  fds_state = FDS_OFF;

  // remove disk
  HAL_GPIO_WritePin(FDS_MEDIA_SET_GPIO_Port, FDS_MEDIA_SET_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(FDS_READY_GPIO_Port, FDS_READY_Pin, GPIO_PIN_SET);

  fds_used_space = 0;
  fds_block_count = 0;
  fds_changed = 0;
}
