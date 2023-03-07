#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include <stdint.h>

typedef struct DotMatrixFont {
  const uint8_t font_type;
	const uint8_t start_char;
	const uint8_t font_length;
	const uint8_t char_width;
	const uint8_t char_height;
	const uint8_t spacing;
	const uint8_t space_length;
	const void* font_data;
} DotMatrixFont;

#include "fonts/fontexterns.h"

#endif /* INC_FONTS_H_ */
