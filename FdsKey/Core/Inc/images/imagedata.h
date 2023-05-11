#include "card_1a.h"
#include "card_1b.h"
#include "card_2a.h"
#include "card_2b.h"
#include "card_3a.h"
#include "card_3b.h"
#include "card_4a.h"
#include "card_4b.h"
#include "card_a.h"
#include "card_b.h"
#include "card_single.h"
#include "card_unknown.h"
#include "cursor.h"
#include "cursor_down.h"
#include "cursor_down_w.h"
#include "cursor_up.h"
#include "cursor_up_w.h"
#include "disk_flip_frame_0.h"
#include "disk_flip_frame_1.h"
#include "disk_flip_frame_2.h"
#include "disk_flip_frame_3.h"
#include "disk_flip_frame_4.h"
#include "disk_flip_frame_5.h"
#include "disk_flip_frame_6.h"
#include "disk_flip_frame_7.h"
#include "disk_flip_frame_8.h"
#include "disk_flip_frame_9.h"
#include "folder6.h"
#include "head_cursor.h"
#include "large_cursor.h"
#include "medium_cursor.h"
#include "microsd_hor.h"
#include "state_ff.h"
#include "state_pause.h"
#include "state_play.h"
#include "state_rec.h"
#include "state_rewind.h"
#include "../images.h"

const DotMatrixImage IMAGE_CARD_1A = {
  .width = IMAGE_CARD_1A_WIDTH,
  .height = IMAGE_CARD_1A_HEIGHT,
  .image_data = image_card_1a
};

const DotMatrixImage IMAGE_CARD_1B = {
  .width = IMAGE_CARD_1B_WIDTH,
  .height = IMAGE_CARD_1B_HEIGHT,
  .image_data = image_card_1b
};

const DotMatrixImage IMAGE_CARD_2A = {
  .width = IMAGE_CARD_2A_WIDTH,
  .height = IMAGE_CARD_2A_HEIGHT,
  .image_data = image_card_2a
};

const DotMatrixImage IMAGE_CARD_2B = {
  .width = IMAGE_CARD_2B_WIDTH,
  .height = IMAGE_CARD_2B_HEIGHT,
  .image_data = image_card_2b
};

const DotMatrixImage IMAGE_CARD_3A = {
  .width = IMAGE_CARD_3A_WIDTH,
  .height = IMAGE_CARD_3A_HEIGHT,
  .image_data = image_card_3a
};

const DotMatrixImage IMAGE_CARD_3B = {
  .width = IMAGE_CARD_3B_WIDTH,
  .height = IMAGE_CARD_3B_HEIGHT,
  .image_data = image_card_3b
};

const DotMatrixImage IMAGE_CARD_4A = {
  .width = IMAGE_CARD_4A_WIDTH,
  .height = IMAGE_CARD_4A_HEIGHT,
  .image_data = image_card_4a
};

const DotMatrixImage IMAGE_CARD_4B = {
  .width = IMAGE_CARD_4B_WIDTH,
  .height = IMAGE_CARD_4B_HEIGHT,
  .image_data = image_card_4b
};

const DotMatrixImage IMAGE_CARD_A = {
  .width = IMAGE_CARD_A_WIDTH,
  .height = IMAGE_CARD_A_HEIGHT,
  .image_data = image_card_a
};

const DotMatrixImage IMAGE_CARD_B = {
  .width = IMAGE_CARD_B_WIDTH,
  .height = IMAGE_CARD_B_HEIGHT,
  .image_data = image_card_b
};

const DotMatrixImage IMAGE_CARD_SINGLE = {
  .width = IMAGE_CARD_SINGLE_WIDTH,
  .height = IMAGE_CARD_SINGLE_HEIGHT,
  .image_data = image_card_single
};

const DotMatrixImage IMAGE_CARD_UNKNOWN = {
  .width = IMAGE_CARD_UNKNOWN_WIDTH,
  .height = IMAGE_CARD_UNKNOWN_HEIGHT,
  .image_data = image_card_unknown
};

const DotMatrixImage IMAGE_CURSOR = {
  .width = IMAGE_CURSOR_WIDTH,
  .height = IMAGE_CURSOR_HEIGHT,
  .image_data = image_cursor
};

const DotMatrixImage IMAGE_CURSOR_DOWN = {
  .width = IMAGE_CURSOR_DOWN_WIDTH,
  .height = IMAGE_CURSOR_DOWN_HEIGHT,
  .image_data = image_cursor_down
};

const DotMatrixImage IMAGE_CURSOR_DOWN_W = {
  .width = IMAGE_CURSOR_DOWN_W_WIDTH,
  .height = IMAGE_CURSOR_DOWN_W_HEIGHT,
  .image_data = image_cursor_down_w
};

const DotMatrixImage IMAGE_CURSOR_UP = {
  .width = IMAGE_CURSOR_UP_WIDTH,
  .height = IMAGE_CURSOR_UP_HEIGHT,
  .image_data = image_cursor_up
};

const DotMatrixImage IMAGE_CURSOR_UP_W = {
  .width = IMAGE_CURSOR_UP_W_WIDTH,
  .height = IMAGE_CURSOR_UP_W_HEIGHT,
  .image_data = image_cursor_up_w
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_0 = {
  .width = IMAGE_DISK_FLIP_FRAME_0_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_0_HEIGHT,
  .image_data = image_disk_flip_frame_0
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_1 = {
  .width = IMAGE_DISK_FLIP_FRAME_1_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_1_HEIGHT,
  .image_data = image_disk_flip_frame_1
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_2 = {
  .width = IMAGE_DISK_FLIP_FRAME_2_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_2_HEIGHT,
  .image_data = image_disk_flip_frame_2
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_3 = {
  .width = IMAGE_DISK_FLIP_FRAME_3_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_3_HEIGHT,
  .image_data = image_disk_flip_frame_3
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_4 = {
  .width = IMAGE_DISK_FLIP_FRAME_4_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_4_HEIGHT,
  .image_data = image_disk_flip_frame_4
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_5 = {
  .width = IMAGE_DISK_FLIP_FRAME_5_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_5_HEIGHT,
  .image_data = image_disk_flip_frame_5
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_6 = {
  .width = IMAGE_DISK_FLIP_FRAME_6_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_6_HEIGHT,
  .image_data = image_disk_flip_frame_6
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_7 = {
  .width = IMAGE_DISK_FLIP_FRAME_7_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_7_HEIGHT,
  .image_data = image_disk_flip_frame_7
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_8 = {
  .width = IMAGE_DISK_FLIP_FRAME_8_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_8_HEIGHT,
  .image_data = image_disk_flip_frame_8
};

const DotMatrixImage IMAGE_DISK_FLIP_FRAME_9 = {
  .width = IMAGE_DISK_FLIP_FRAME_9_WIDTH,
  .height = IMAGE_DISK_FLIP_FRAME_9_HEIGHT,
  .image_data = image_disk_flip_frame_9
};

const DotMatrixImage IMAGE_FOLDER6 = {
  .width = IMAGE_FOLDER6_WIDTH,
  .height = IMAGE_FOLDER6_HEIGHT,
  .image_data = image_folder6
};

const DotMatrixImage IMAGE_HEAD_CURSOR = {
  .width = IMAGE_HEAD_CURSOR_WIDTH,
  .height = IMAGE_HEAD_CURSOR_HEIGHT,
  .image_data = image_head_cursor
};

const DotMatrixImage IMAGE_LARGE_CURSOR = {
  .width = IMAGE_LARGE_CURSOR_WIDTH,
  .height = IMAGE_LARGE_CURSOR_HEIGHT,
  .image_data = image_large_cursor
};

const DotMatrixImage IMAGE_MEDIUM_CURSOR = {
  .width = IMAGE_MEDIUM_CURSOR_WIDTH,
  .height = IMAGE_MEDIUM_CURSOR_HEIGHT,
  .image_data = image_medium_cursor
};

const DotMatrixImage IMAGE_MICROSD_HOR = {
  .width = IMAGE_MICROSD_HOR_WIDTH,
  .height = IMAGE_MICROSD_HOR_HEIGHT,
  .image_data = image_microsd_hor
};

const DotMatrixImage IMAGE_STATE_FF = {
  .width = IMAGE_STATE_FF_WIDTH,
  .height = IMAGE_STATE_FF_HEIGHT,
  .image_data = image_state_ff
};

const DotMatrixImage IMAGE_STATE_PAUSE = {
  .width = IMAGE_STATE_PAUSE_WIDTH,
  .height = IMAGE_STATE_PAUSE_HEIGHT,
  .image_data = image_state_pause
};

const DotMatrixImage IMAGE_STATE_PLAY = {
  .width = IMAGE_STATE_PLAY_WIDTH,
  .height = IMAGE_STATE_PLAY_HEIGHT,
  .image_data = image_state_play
};

const DotMatrixImage IMAGE_STATE_REC = {
  .width = IMAGE_STATE_REC_WIDTH,
  .height = IMAGE_STATE_REC_HEIGHT,
  .image_data = image_state_rec
};

const DotMatrixImage IMAGE_STATE_REWIND = {
  .width = IMAGE_STATE_REWIND_WIDTH,
  .height = IMAGE_STATE_REWIND_HEIGHT,
  .image_data = image_state_rewind
};
