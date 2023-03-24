#include "standard_6_font.h"
#include "verdana_12_bold_font.h"
#include "../fonts.h"

const DotMatrixFont FONT_STANDARD_6 = {
  .font_type = FONT_STANDARD_6_FONT_TYPE,
  .start_char = FONT_STANDARD_6_START_CHAR,
  .font_length = FONT_STANDARD_6_LENGTH,
  .char_width = FONT_STANDARD_6_CHAR_WIDTH,
  .char_height = FONT_STANDARD_6_CHAR_HEIGHT,
  .spacing = 1,
  .space_length = 0,
  .font_data = font_standard_6
};

const DotMatrixFont FONT_VERDANA_12_BOLD = {
  .font_type = FONT_VERDANA_12_BOLD_FONT_TYPE,
  .start_char = FONT_VERDANA_12_BOLD_START_CHAR,
  .font_length = FONT_VERDANA_12_BOLD_LENGTH,
  .char_width = FONT_VERDANA_12_BOLD_CHAR_WIDTH,
  .char_height = FONT_VERDANA_12_BOLD_CHAR_HEIGHT,
  .spacing = 1,
  .space_length = 0,
  .font_data = font_verdana_12_bold
};
