#ifndef INC_FDSEMU_H_
#define INC_FDSEMU_H_

#include "fatfs.h"

// hardware settings
#define FDS_READ_PWM_TIMER htim3
#define FDS_READ_PWM_TIMER_CHANNEL 2
#define FDS_READ_DMA hdma_tim3_up
#define FDS_READ_IMPULSE_LENGTH 32

#define FDS_WRITE_CAPTURE_TIMER htim15
#define FDS_WRITE_CAPTURE_TIMER_CHANNEL 1
#define FDS_WRITE_DMA hdma_tim15_ch1
#define FDS_THRESHOLD_1 960
#define FDS_THRESHOLD_2 1120

// FDS emulation settings
#define FDS_MAX_SIDE_SIZE (80 * 1024) // 65000 + some space for gaps and crcs
#define FDS_MAX_FILE_PATH_LENGTH 4096
#define FDS_MAX_BLOCKS 256
#define FDS_MAX_BLOCK_SIZE FDS_MAX_SIDE_SIZE
#define FDS_READ_BUFFER_SIZE 128      // bits
#define FDS_WRITE_BUFFER_SIZE 32     // impulses
#define FDS_FIRST_GAP_READ_BITS 28300 // first gap size, bits
#define FDS_NEXT_GAPS_READ_BITS 976   // next gap size, bits
#define FDS_WRITE_GAP_SKIP_BITS 32     // dispose bits before writing
#define FDS_NOT_READY_TIME 1000       // disk rewind time, milliseconds (for fast rewind)
#define FDS_NOT_READY_BYTES 16000      // disk rewind time, bytes (for slow rewind)
#define FDS_MULTI_WRITE_UNLICENSED_BITS 32 // some unlicensed software can write multiple blocks at once

// do not touch it
#define FDS_HEADER_SIZE 16    // header in ROM
#define FDS_SIDE_SIZE 65500   // disk side size in ROM

// special subdefines
#define FDS_GLUE(a, b) a##b
#define FDS_TIMER_CHANNEL_REG(v) FDS_GLUE(CCR, v)
#define FDS_TIMER_CHANNEL_CONST(v) FDS_GLUE(TIM_CHANNEL_, v)
#define FDS_TIMER_DMA_TRIGGER_CONST(v) FDS_GLUE(TIM_DMA_CC, v)
#define FDS_READ_PWM_TIMER_CHANNEL_REG FDS_TIMER_CHANNEL_REG(FDS_READ_PWM_TIMER_CHANNEL)
#define FDS_READ_PWM_TIMER_CHANNEL_CONST FDS_TIMER_CHANNEL_CONST(FDS_READ_PWM_TIMER_CHANNEL)
#define FDS_WRITE_CAPTURE_TIMER_CHANNEL_REG FDS_TIMER_CHANNEL_REG(FDS_WRITE_CAPTURE_TIMER_CHANNEL)
#define FDS_WRITE_CAPTURE_TIMER_CHANNEL_CONST FDS_TIMER_CHANNEL_CONST(FDS_WRITE_CAPTURE_TIMER_CHANNEL)
#define FDS_WRITE_CAPTURE_DMA_TRIGGER_CONST FDS_TIMER_DMA_TRIGGER_CONST(FDS_WRITE_CAPTURE_TIMER_CHANNEL)

typedef enum {
  FDS_OFF,                    // disk image is not loaded
  FDS_IDLE,                   // disk stopped
  FDS_READ_WAIT_READY,        // waiting for disk rewinding
  FDS_READ_WAIT_READY_TIMER,  // not-ready timer
  FDS_READING,                // reading data
  FDS_WRITING_GAP,            // waiting for gap terminator before writing
  FDS_WRITING,                // writing data
  FDS_WRITING_STOPPING,       // end of useful data, writing garbage
  FDS_SAVING                  // saving image
} FDS_STATE;

#define FDSR_WRONG_CRC 0x80
#define FDSR_INVALID_ROM 0x81

FRESULT fds_load_side(char *filename, uint8_t side);
FRESULT fds_close(uint8_t save, uint8_t backup_original);
FRESULT fds_save(uint8_t backup_original);
FRESULT fds_get_sides_count(char *filename, uint8_t *count);
void fds_check_pins();
void fds_tick_100ms();
uint8_t fds_is_changed();
FDS_STATE fds_get_state();

extern TIM_HandleTypeDef FDS_READ_PWM_TIMER;
extern DMA_HandleTypeDef FDS_READ_DMA;
extern TIM_HandleTypeDef FDS_WRITE_TIMER;

extern DMA_HandleTypeDef hdma_tim15_ch1;
extern TIM_HandleTypeDef htim15;

#endif /* INC_FDSEMU_H_ */
