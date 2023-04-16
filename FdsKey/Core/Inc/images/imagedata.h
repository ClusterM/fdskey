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
#include "card_c.h"
#include "card_d.h"
#include "card_e.h"
#include "card_f.h"
#include "card_g.h"
#include "card_h.h"
#include "card_single.h"
#include "card_unknown.h"
#include "cursor.h"
#include "cursor_down.h"
#include "cursor_down_w.h"
#include "cursor_up.h"
#include "cursor_up_w.h"
#include "folder.h"
#include "folder2.h"
#include "folder3.h"
#include "folder4.h"
#include "folder5.h"
#include "folder6.h"
#include "folder_up.h"
#include "head_cursor.h"
#include "large_cursor.h"
#include "medium_cursor.h"
#include "microsd.h"
#include "microsd2.h"
#include "microsd_hor.h"
#include "microsd_hor2.h"
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

const DotMatrixImage IMAGE_CARD_C = {
  .width = IMAGE_CARD_C_WIDTH,
  .height = IMAGE_CARD_C_HEIGHT,
  .image_data = image_card_c
};

const DotMatrixImage IMAGE_CARD_D = {
  .width = IMAGE_CARD_D_WIDTH,
  .height = IMAGE_CARD_D_HEIGHT,
  .image_data = image_card_d
};

const DotMatrixImage IMAGE_CARD_E = {
  .width = IMAGE_CARD_E_WIDTH,
  .height = IMAGE_CARD_E_HEIGHT,
  .image_data = image_card_e
};

const DotMatrixImage IMAGE_CARD_F = {
  .width = IMAGE_CARD_F_WIDTH,
  .height = IMAGE_CARD_F_HEIGHT,
  .image_data = image_card_f
};

const DotMatrixImage IMAGE_CARD_G = {
  .width = IMAGE_CARD_G_WIDTH,
  .height = IMAGE_CARD_G_HEIGHT,
  .image_data = image_card_g
};

const DotMatrixImage IMAGE_CARD_H = {
  .width = IMAGE_CARD_H_WIDTH,
  .height = IMAGE_CARD_H_HEIGHT,
  .image_data = image_card_h
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

const DotMatrixImage IMAGE_FOLDER = {
  .width = IMAGE_FOLDER_WIDTH,
  .height = IMAGE_FOLDER_HEIGHT,
  .image_data = image_folder
};

const DotMatrixImage IMAGE_FOLDER2 = {
  .width = IMAGE_FOLDER2_WIDTH,
  .height = IMAGE_FOLDER2_HEIGHT,
  .image_data = image_folder2
};

const DotMatrixImage IMAGE_FOLDER3 = {
  .width = IMAGE_FOLDER3_WIDTH,
  .height = IMAGE_FOLDER3_HEIGHT,
  .image_data = image_folder3
};

const DotMatrixImage IMAGE_FOLDER4 = {
  .width = IMAGE_FOLDER4_WIDTH,
  .height = IMAGE_FOLDER4_HEIGHT,
  .image_data = image_folder4
};

const DotMatrixImage IMAGE_FOLDER5 = {
  .width = IMAGE_FOLDER5_WIDTH,
  .height = IMAGE_FOLDER5_HEIGHT,
  .image_data = image_folder5
};

const DotMatrixImage IMAGE_FOLDER6 = {
  .width = IMAGE_FOLDER6_WIDTH,
  .height = IMAGE_FOLDER6_HEIGHT,
  .image_data = image_folder6
};

const DotMatrixImage IMAGE_FOLDER_UP = {
  .width = IMAGE_FOLDER_UP_WIDTH,
  .height = IMAGE_FOLDER_UP_HEIGHT,
  .image_data = image_folder_up
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

const DotMatrixImage IMAGE_MICROSD = {
  .width = IMAGE_MICROSD_WIDTH,
  .height = IMAGE_MICROSD_HEIGHT,
  .image_data = image_microsd
};

const DotMatrixImage IMAGE_MICROSD2 = {
  .width = IMAGE_MICROSD2_WIDTH,
  .height = IMAGE_MICROSD2_HEIGHT,
  .image_data = image_microsd2
};

const DotMatrixImage IMAGE_MICROSD_HOR = {
  .width = IMAGE_MICROSD_HOR_WIDTH,
  .height = IMAGE_MICROSD_HOR_HEIGHT,
  .image_data = image_microsd_hor
};

const DotMatrixImage IMAGE_MICROSD_HOR2 = {
  .width = IMAGE_MICROSD_HOR2_WIDTH,
  .height = IMAGE_MICROSD_HOR2_HEIGHT,
  .image_data = image_microsd_hor2
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
