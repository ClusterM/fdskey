#include "card_a.h"
#include "card_b.h"
#include "../images.h"

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
