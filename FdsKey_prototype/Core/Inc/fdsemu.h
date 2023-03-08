#ifndef INC_FDSEMU_H_
#define INC_FDSEMU_H_

#include "fatfs.h"

// hardware settings
#define FDS_READ_PWM_TIMER htim3
#define FDS_READ_PWM_TIMER_CHANNEL 2
#define FDS_READ_DMA hdma_tim3_up
#define FDS_READ_IMPULSE_LENGTH 32
#define FDS_WRITE_TIMER htim2

// FDS emulation settings
#define FDS_MAX_SIDE_SIZE (80 * 1024) // 65000 + some space for gaps and crcs
#define FDS_MAX_FILE_PATH_LENGHT 4096
#define FDS_MAX_BLOCKS 256
#define FDS_MAX_BLOCK_SIZE FDS_MAX_SIDE_SIZE
#define FDS_READ_BUFFER_SIZE 128 // bits
#define FDS_FIRST_GAP_READ_BITS 28300
#define FDS_NEXT_GAPS_READ_BITS 976
#define FDS_WRITE_GAP_SKIP_BITS 4
#define FDS_THRESHOLD_1 960
#define FDS_THRESHOLD_2 1120
#define FDS_NOT_READY_TIME 1000 // milliseconds
#define FDS_NOT_READY_BYTES 4096

// do not touch it
#define FDS_HEADER_SIZE 16    // heade in ROM
#define FDS_SIDE_SIZE 65500 // disk side size in ROM

// special subdefines
#define FDS_GLUE(a, b) a##b
#define FDS_PWM_TIMER_CHANNEL_REG(v) FDS_GLUE(CCR, v)
#define FDS_PWM_TIMER_CHANNEL_CONST(v) FDS_GLUE(TIM_CHANNEL_, v)
#define FDS_READ_PWM_TIMER_CHANNEL_REG FDS_PWM_TIMER_CHANNEL_REG(FDS_READ_PWM_TIMER_CHANNEL)
#define FDS_READ_PWM_TIMER_CHANNEL_CONST FDS_PWM_TIMER_CHANNEL_CONST(FDS_READ_PWM_TIMER_CHANNEL)

typedef enum {
  FDS_OFF,
  FDS_IDLE,
  FDS_READ_WAIT_READY,
  FDS_READ_WAIT_READY_TIMER,
  FDS_READING,
  FDS_WRITING_GAP,
  FDS_WRITING,
  FDS_SAVING
} FDS_STATE;

FRESULT fds_load_side(char *filename, uint8_t side);
FRESULT fds_close(uint8_t save, uint8_t backup_original);
FRESULT fds_save(uint8_t backup_original);
FRESULT fds_get_sides_count(char *filename, uint8_t *count);
void fds_write_impulse();
void fds_check_pins();
void fds_tick_100ms();
uint8_t fds_is_changed();
FDS_STATE fds_get_state();

extern TIM_HandleTypeDef FDS_READ_PWM_TIMER;
extern DMA_HandleTypeDef FDS_READ_DMA;
extern TIM_HandleTypeDef FDS_WRITE_TIMER;

#endif /* INC_FDSEMU_H_ */
