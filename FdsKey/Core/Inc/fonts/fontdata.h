#include "digits_font.h"
#include "gamegirl_classic_12_font.h"
#include "gamegirl_classic_6_font.h"
#include "slimfont_8_font.h"
#include "standard_6_font.h"
#include "ultraslimfont_8_font.h"
#include "verdana_12_bold_font.h"
#include "verdana_14_bold_font.h"
#include "../fonts.h"

const DotMatrixFont FONT_DIGITS = {
  .font_type = FONT_DIGITS_FONT_TYPE,
  .start_char = FONT_DIGITS_START_CHAR,
  .font_length = FONT_DIGITS_LENGTH,
  .char_width = FONT_DIGITS_CHAR_WIDTH,
  .char_height = FONT_DIGITS_CHAR_HEIGHT,
  .spacing = 1,
  .space_length = 0,
  .font_data = font_digits
};

const DotMatrixFont FONT_GAMEGIRL_CLASSIC_12 = {
  .font_type = FONT_GAMEGIRL_CLASSIC_12_FONT_TYPE,
  .start_char = FONT_GAMEGIRL_CLASSIC_12_START_CHAR,
  .font_length = FONT_GAMEGIRL_CLASSIC_12_LENGTH,
  .char_width = FONT_GAMEGIRL_CLASSIC_12_CHAR_WIDTH,
  .char_height = FONT_GAMEGIRL_CLASSIC_12_CHAR_HEIGHT,
  .spacing = 0,
  .space_length = 0,
  .font_data = font_gamegirl_classic_12
};

const DotMatrixFont FONT_GAMEGIRL_CLASSIC_6 = {
  .font_type = FONT_GAMEGIRL_CLASSIC_6_FONT_TYPE,
  .start_char = FONT_GAMEGIRL_CLASSIC_6_START_CHAR,
  .font_length = FONT_GAMEGIRL_CLASSIC_6_LENGTH,
  .char_width = FONT_GAMEGIRL_CLASSIC_6_CHAR_WIDTH,
  .char_height = FONT_GAMEGIRL_CLASSIC_6_CHAR_HEIGHT,
  .spacing = 0,
  .space_length = 0,
  .font_data = font_gamegirl_classic_6
};

const DotMatrixFont FONT_SLIMFONT_8 = {
  .font_type = FONT_SLIMFONT_8_FONT_TYPE,
  .start_char = FONT_SLIMFONT_8_START_CHAR,
  .font_length = FONT_SLIMFONT_8_LENGTH,
  .char_width = FONT_SLIMFONT_8_CHAR_WIDTH,
  .char_height = FONT_SLIMFONT_8_CHAR_HEIGHT,
  .spacing = 1,
  .space_length = 3,
  .font_data = font_slimfont_8
};

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

const DotMatrixFont FONT_ULTRASLIMFONT_8 = {
  .font_type = FONT_ULTRASLIMFONT_8_FONT_TYPE,
  .start_char = FONT_ULTRASLIMFONT_8_START_CHAR,
  .font_length = FONT_ULTRASLIMFONT_8_LENGTH,
  .char_width = FONT_ULTRASLIMFONT_8_CHAR_WIDTH,
  .char_height = FONT_ULTRASLIMFONT_8_CHAR_HEIGHT,
  .spacing = 1,
  .space_length = 2,
  .font_data = font_ultraslimfont_8
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

const DotMatrixFont FONT_VERDANA_14_BOLD = {
  .font_type = FONT_VERDANA_14_BOLD_FONT_TYPE,
  .start_char = FONT_VERDANA_14_BOLD_START_CHAR,
  .font_length = FONT_VERDANA_14_BOLD_LENGTH,
  .char_width = FONT_VERDANA_14_BOLD_CHAR_WIDTH,
  .char_height = FONT_VERDANA_14_BOLD_CHAR_HEIGHT,
  .spacing = 1,
  .space_length = 0,
  .font_data = font_verdana_14_bold
};
