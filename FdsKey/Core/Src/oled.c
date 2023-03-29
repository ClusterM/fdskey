#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "main.h"
#include "oled.h"

static uint8_t rotate = 0;
static uint8_t padding_left = 0;
static uint8_t padding_top = 0;
static uint8_t current_line = 0;
static uint8_t image[OLED_HEIGHT * 2 * OLED_WIDTH];

// init OLED and buffer
void oled_init( uint8_t rotate_screen, uint8_t reverse, uint8_t contrast)
{
  // padding_left depends on OLED controller version
  // can be SH1106 (132x64) or SSD1306 (128x64)
  // TODO: make display controller selection
	padding_left = 0; // rotate_screen ? 0 : 4;
	padding_top = rotate_screen ? 32 : 0;
	rotate = rotate_screen;

	oled_send_commands(11,
	OLED_CMD_SET_OFF,
    0x8D, 0x14, // enable charge pump (???)
    reverse ? OLED_CMD_SET_REVERSE_ON : OLED_CMD_SET_REVERSE_OFF,
    OLED_CMD_SET_START_LINE(current_line + padding_top),
    OLED_CMD_SET_PADS_MODE, OLED_CMD_SET_PADS_MODE_SEQUENTIAL,
    rotate_screen ?
        OLED_CMD_SET_VERTICAL_FLIP_ON :
        OLED_CMD_SET_VERTICAL_FLIP_OFF,
    rotate_screen ?
        OLED_CMD_SET_HORIZONTAL_FLIP_ON :
        OLED_CMD_SET_HORIZONTAL_FLIP_OFF,
    OLED_CMD_SET_CONTRAST_MODE, contrast);
	memset(image, 0, sizeof(image));
	oled_update_full();
	oled_send_commands(1, OLED_CMD_SET_ON);
}

// return pointer to a pixel
uint8_t* oled_pixel(int x, int y) {
	return image + (y % (OLED_HEIGHT * 2)) * OLED_WIDTH + (x % OLED_WIDTH);
}

// set single pixel
void oled_set_pixel(int x, int y, uint8_t value) {
	*oled_pixel(x, y) = value;
}

// get pixel value
uint8_t oled_get_pixel(int x, int y) {
	return *oled_pixel(x, y);
}

// send commads to OLED controller
HAL_StatusTypeDef oled_send_commands(int len, ...) {
	uint8_t buffer[len * 2];
	va_list valist;
	va_start(valist, len);

	int i;
	for (i = 0; i < len; i++) {
		buffer[i * 2] = i + 1 < len ? OLED_COMMAND_NOT_LAST : OLED_COMMAND_LAST;
		buffer[i * 2 + 1] = (uint8_t) va_arg(valist, int);
	}
	HAL_StatusTypeDef r = HAL_I2C_Master_Transmit(&OLED_I2C, OLED_ADDRESS << 1, buffer,
			sizeof(buffer), OLED_TIMEOUT);
	return r;
}

// send single command to OLED controller
HAL_StatusTypeDef oled_send_command(uint8_t command) {
	return oled_send_commands(1, command);
}

// write data to OLED
HAL_StatusTypeDef oled_write_data(uint8_t *data, uint8_t len) {
	uint8_t buffer[len + 1];
	buffer[0] = OLED_COMMAND_DATA;
	memcpy(&buffer[1], data, len);
	return HAL_I2C_Master_Transmit(&OLED_I2C, OLED_ADDRESS << 1, buffer, sizeof(buffer),
			OLED_TIMEOUT);
}

// transfer data from our buffer to OLED buffer
HAL_StatusTypeDef oled_update(uint8_t start_page, uint8_t end_page) {
	uint8_t p, x, y, l, bt, buffer[256], bpos;
	HAL_StatusTypeDef r = HAL_OK;

	start_page = start_page % (OLED_HEIGHT * 2 / 8);
	end_page = end_page % (OLED_HEIGHT * 2 / 8);
	if (end_page < start_page) end_page += (OLED_HEIGHT * 2 / 8);

	for (p = start_page; p < end_page + 1; p++) {
		oled_send_commands(3, OLED_CMD_SET_PAGE(p % 8),
				OLED_CMD_SET_COLUMN_LOW(padding_left),
				OLED_CMD_SET_COLUMN_HIGH(padding_left));
		bpos = 0;
		for (x = 0; x < OLED_WIDTH; x++) {
			bt = 0;
			for (l = 0; l < 8; l++) {
				y = (p * 8 + l) % (OLED_HEIGHT * 2);
				bt >>= 1;
				if (*oled_pixel(x, y))
					bt |= 0x80;
			}
			buffer[bpos++] = bt;
			if (bpos >= sizeof(buffer) || x == OLED_WIDTH - 1) {
				r = oled_write_data(buffer, bpos);
				if (r != HAL_OK)
					return r;
				bpos = 0;
			}
		}
	}
	return r;
}

// transfer all data from our buffer to OLED buffer
HAL_StatusTypeDef oled_update_full() {
	return oled_update(0, OLED_HEIGHT * 2 / 8 - 1);
}

// transfer invisible data from our buffer to OLED buffer
HAL_StatusTypeDef oled_update_invisible() {
	uint8_t start_page = ((current_line + OLED_HEIGHT) % (OLED_HEIGHT * 2)) / 8;
	uint8_t end_page = start_page
			+ ((current_line % 8 == 0) ? OLED_HEIGHT / 8 - 1 : OLED_HEIGHT / 8);
	return oled_update(start_page, end_page);
}

// scroll to line
HAL_StatusTypeDef oled_set_line(int y) {
	current_line = y % (OLED_HEIGHT * 2);
	return oled_send_command(
			OLED_CMD_SET_START_LINE(current_line + padding_top));
}

// get current scrolling
uint8_t oled_get_line() {
	return current_line;
}

// copy visible buffer to invisible
void oled_copy_to_invisible() {
	int y;
	for (y = 0; y < OLED_HEIGHT; y++) {
		memcpy(oled_pixel(0, current_line + y + OLED_HEIGHT),
				oled_pixel(0, current_line + y), OLED_WIDTH);
	}
}

// switch OLED to invisible buffer
void oled_switch_to_invisible() {
	oled_set_line(current_line + OLED_HEIGHT);
}

// draw rectangle
void oled_draw_rectangle(int x1, int y1, int x2, int y2,
		uint8_t fill, uint8_t value) {
	int x, y;

	if (x2 < x1) {
		x = x1;
		x1 = x2;
		x2 = x;
	}
	if (y2 < y1) {
		y = y1;
		y1 = y2;
		y2 = y;
	}

	for (y = y1; y <= y2; y++) {
		if (fill || y == y1 || y == y2) {
			for (x = x1; x <= x2; x++) {
				*oled_pixel(x, y) = value;
			}
		} else {
			*oled_pixel(x1, y) = *oled_pixel(x2, y) = value;
		}
	}
}

// draw line
void oled_draw_line(int x1, int y1, int x2, int y2,
		uint8_t value) {
	int x, y;

	if (x1 == x2 || y1 == y2) {
		oled_draw_rectangle(x1, y1, x2, y2, 0, value);
	} else if (abs(x2 - x1) >= abs(y2 - y1)) {
		if (x2 > x1)
			for (x = x1; x <= x2; x++)
				*oled_pixel(x,
						(int) round(
								(float) y1
										+ ((float) y2 - (float) y1)
												* ((float) x - (float) x1)
												/ ((float) x2 - (float) x1))) =
						value;
		else
			for (x = x2; x <= x1; x++)
				*oled_pixel(x,
						(int) round(
								(float) y2
										+ ((float) y2 - (float) y1)
												* ((float) x2 - (float) x)
												/ ((float) x1 - (float) x2))) =
						value;
	} else {
		if (y2 > y1)
			for (y = y1; y <= y2; y++)
				*oled_pixel(
						(int) round(
								(float) x1
										+ ((float) x2 - (float) x1)
												* ((float) y - (float) y1)
												/ ((float) y2 - (float) y1)), y) =
						value;
		else
			for (y = y2; y <= y1; y++)
				*oled_pixel(
						(int) round(
								(float) x2
										+ ((float) x2 - (float) x1)
												* ((float) y2 - (float) y)
												/ ((float) y1 - (float) y2)), y) =
						value;
	}
}

// draw text
void oled_draw_text(const DotMatrixFont *font, char *text, int x, int y,
		uint8_t replace, uint8_t invert) {
	oled_draw_text_cropped(font, text, x, y, 0, 0, 0, 0, replace, invert);
}

// draw cropped text
void oled_draw_text_cropped(const DotMatrixFont *font, char *text, int x, int y,
		uint8_t start_x, uint8_t max_width, uint8_t start_y, uint8_t max_height,
		uint8_t replace, uint8_t invert) {
	int xpos = x; // current x position
	int len = 0; // total text length
	int c, l; // column/line of current character
	int xp, yp; // temporary variables for dot coordinates
	uint8_t *char_data; // pointer for character data
	uint64_t char_data_casted; // unboxed character data
	uint8_t char_width; // current character width
  uint8_t size_offset = font->font_type;
	while (*text) {
		char ch = *text;
		// replace unknown characters with underscore
		if (ch < font->start_char || ch >= font->start_char + font->font_length)
			ch = '_';

		// get character length
		if (font->char_height <= 8)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width + size_offset)];
		else if (font->char_height <= 16)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * sizeof(uint16_t) + size_offset)];
		else if (font->char_height <= 24)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * 3 + size_offset)];
		else if (font->char_height <= 32)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * sizeof(uint32_t) + size_offset)];
		else if (font->char_height <= 40)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * 5 + size_offset)];
		else if (font->char_height <= 48)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * 6 + size_offset)];
		else if (font->char_height <= 56)
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * 7 + size_offset)];
		else
			char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
					* (font->char_width * sizeof(uint64_t) + size_offset)];

		// resize spaces if need
		if (font->font_type == 0) // monospaced
		{
		  char_width = font->char_width;
		} else {
      char_width =
          (ch != ' ' || !font->space_length) ?
              char_data[0] : font->space_length;
		}

		// for every column of character
		for (c = 0; c < char_width; c++) {
			// stop if maximum length meet
			if (max_width && len - start_x > max_width)
				return;
			// skip columns
			if (start_x && start_x > len) {
				len++;
				continue;
			}

			// unbox character data
      if (font->char_height <= 8)
        char_data_casted = *((uint8_t*) (char_data + c + size_offset));
      else if (font->char_height <= 16)
      {
        uint8_t b1 = *(char_data + c * 2 + size_offset);
        uint8_t b2 = *(char_data + c * 2 + size_offset + 1);
        char_data_casted = b1 | (b2 << 8);
      } else if (font->char_height <= 24) {
        uint8_t b1 = *(char_data + c * 3 + size_offset);
        uint8_t b2 = *(char_data + c * 3 + size_offset + 1);
        uint8_t b3 = *(char_data + c * 3 + size_offset + 2);
        char_data_casted = b1 | (b2 << 8) | (b3 << 16);
      } else if (font->char_height <= 32) {
        uint8_t b1 = *(char_data + c * 4 + size_offset);
        uint8_t b2 = *(char_data + c * 4 + size_offset + 1);
        uint8_t b3 = *(char_data + c * 4 + size_offset + 2);
        uint8_t b4 = *(char_data + c * 4 + size_offset + 3);
        char_data_casted = b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
      } else if (font->char_height <= 40) {
        uint8_t b1 = *(char_data + c * 5 + size_offset);
        uint8_t b2 = *(char_data + c * 5 + size_offset + 1);
        uint8_t b3 = *(char_data + c * 5 + size_offset + 2);
        uint8_t b4 = *(char_data + c * 5 + size_offset + 3);
        uint8_t b5 = *(char_data + c * 5 + size_offset + 4);
        char_data_casted = (uint64_t)b1 | ((uint64_t)b2 << 8) | ((uint64_t)b3 << 16) | ((uint64_t)b4 << 24) | ((uint64_t)b5 << 32);
      } else if (font->char_height <= 48) {
        uint8_t b1 = *(char_data + c * 6 + size_offset);
        uint8_t b2 = *(char_data + c * 6 + size_offset + 1);
        uint8_t b3 = *(char_data + c * 6 + size_offset + 2);
        uint8_t b4 = *(char_data + c * 6 + size_offset + 3);
        uint8_t b5 = *(char_data + c * 6 + size_offset + 4);
        uint8_t b6 = *(char_data + c * 6 + size_offset + 5);
        char_data_casted = (uint64_t)b1 | ((uint64_t)b2 << 8) | ((uint64_t)b3 << 16) | ((uint64_t)b4 << 24) | ((uint64_t)b5 << 32) | ((uint64_t)b6 << 40);
      } else if (font->char_height <= 56) {
        uint8_t b1 = *(char_data + c * 7 + size_offset);
        uint8_t b2 = *(char_data + c * 7 + size_offset + 1);
        uint8_t b3 = *(char_data + c * 7 + size_offset + 2);
        uint8_t b4 = *(char_data + c * 7 + size_offset + 3);
        uint8_t b5 = *(char_data + c * 7 + size_offset + 4);
        uint8_t b6 = *(char_data + c * 7 + size_offset + 5);
        uint8_t b7 = *(char_data + c * 7 + size_offset + 6);
        char_data_casted = (uint64_t)b1 | ((uint64_t)b2 << 8) | ((uint64_t)b3 << 16) | ((uint64_t)b4 << 24) | ((uint64_t)b5 << 32) | ((uint64_t)b6 << 40) | ((uint64_t)b7 << 48);
      } else {
        uint8_t b1 = *(char_data + c * 8 + size_offset);
        uint8_t b2 = *(char_data + c * 8 + size_offset + 1);
        uint8_t b3 = *(char_data + c * 8 + size_offset + 2);
        uint8_t b4 = *(char_data + c * 8 + size_offset + 3);
        uint8_t b5 = *(char_data + c * 8 + size_offset + 4);
        uint8_t b6 = *(char_data + c * 8 + size_offset + 5);
        uint8_t b7 = *(char_data + c * 8 + size_offset + 6);
        uint8_t b8 = *(char_data + c * 8 + size_offset + 7);
        char_data_casted = (uint64_t)b1 | ((uint64_t)b2 << 8) | ((uint64_t)b3 << 16) | ((uint64_t)b4 << 24) | ((uint64_t)b5 << 32) | ((uint64_t)b6 << 40) | ((uint64_t)b7 << 48) | ((uint64_t)b8 << 56);
      }

			// for every line of character (or until max height)
			for (l = start_y;
					(l < font->char_height)
							&& (!max_height || l - start_y < max_height); l++) {
				xp = xpos + c - start_x;
				yp = y + l - start_y;
				if (char_data_casted & (1 << l))
					*oled_pixel(xp, yp) = !invert;
				else if (replace)
					*oled_pixel(xp, yp) = invert;
			}
			len++;
		}

		text++;
		// fill gap between characters
		if (font->spacing > 0 && replace && *text)
			oled_draw_rectangle(xpos + char_width, y,
					xpos + char_width + font->spacing - 1, y + font->char_height - 1,
					1, invert);
		xpos += char_width + (*text ? font->spacing : 0);
		if (*text)
		  len += font->spacing;
	}
}

// calculate text length
int oled_get_text_length(const DotMatrixFont *font, char *text) {
  int len = 0; // total text length
  uint8_t *char_data; // pointer for character data
  uint8_t char_width; // current character width
  uint8_t size_offset = font->font_type;
  while (*text) {
    char ch = *text;
    // replace unknown characters with underscore
    if (ch < font->start_char || ch >= font->start_char + font->font_length)
      ch = '_';

    // get character length
    if (font->char_height <= 8)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width + size_offset)];
    else if (font->char_height <= 16)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * sizeof(uint16_t) + size_offset)];
    else if (font->char_height <= 24)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * 3 + size_offset)];
    else if (font->char_height <= 32)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * sizeof(uint32_t) + size_offset)];
    else if (font->char_height <= 40)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * 5 + size_offset)];
    else if (font->char_height <= 48)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * 6 + size_offset)];
    else if (font->char_height <= 56)
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * 7 + size_offset)];
    else
      char_data = &((uint8_t*) font->font_data)[(ch - font->start_char)
          * (font->char_width * sizeof(uint64_t) + size_offset)];

    // resize spaces if need
    if (font->font_type == 0) // monospaced
    {
      char_width = font->char_width;
    } else {
      char_width =
          (ch != ' ' || !font->space_length) ?
              char_data[0] : font->space_length;
    }

    text++;
    len += char_width + (*text ? font->spacing : 0);
  }
  return len;
}

// draw image
void oled_draw_image(const DotMatrixImage *img, int x, int y,
    uint8_t replace, uint8_t invert) {
  oled_draw_image_cropped(img, x, y, 0, 0, 0, 0, replace, invert);
  int c, l;
  uint8_t bit = 0;
  int pos = 0;

  for (l = 0; l < img->height; l++) {
    for (c = 0; c < img->width; c++) {
      if (img->image_data[pos] & (1 << bit))
        *oled_pixel(x + c, y + l) = !invert;
      else if (replace)
        *oled_pixel(x + c, y + l) = invert;
      bit++;
      if (bit >= 8) {
        bit = 0;
        pos++;
      }
    }
  }
}

// draw cropped image
void oled_draw_image_cropped(const DotMatrixImage *img, int x, int y,
		uint8_t start_x, uint8_t max_width, uint8_t start_y, uint8_t max_height,
		uint8_t replace, uint8_t invert) {
	int c, l;
	uint8_t bit = 0;
	int pos = 0;
	if (!max_width)
		max_width = img->width;
	if (!max_height)
		max_height = img->height;

	for (l = 0; l < img->height; l++) {
		for (c = 0; c < img->width; c++) {
			if (c >= start_x && l >= start_y && c < max_width + start_x
					&& l < max_height + start_y) {
				if (img->image_data[pos] & (1 << bit))
					*oled_pixel(x + c - start_x, y + l - start_y) = !invert;
				else if (replace)
					*oled_pixel(x + c - start_x, y + l - start_y) = invert;
			}
			bit++;
			if (bit >= 8) {
				bit = 0;
				pos++;
			}
		}
	}
}

// rotate screen, keep buffer and update screen
void oled_rotate(uint8_t rotate_screen)
{
  if (!!rotate == !!rotate_screen)
    return;
  oled_send_command(OLED_CMD_SET_OFF);
  // TODO: OLED controller selection
  padding_left = 0; //rotate_screen ? 0 : 4;
  padding_top = rotate_screen ? 32 : 0;
  oled_send_command(OLED_CMD_SET_START_LINE(current_line + padding_top));
  oled_send_commands(2,
      rotate_screen ?
          OLED_CMD_SET_VERTICAL_FLIP_ON :
          OLED_CMD_SET_VERTICAL_FLIP_OFF,
      rotate_screen ?
          OLED_CMD_SET_HORIZONTAL_FLIP_ON :
          OLED_CMD_SET_HORIZONTAL_FLIP_OFF
  );
  oled_update_full();
  oled_send_command(OLED_CMD_SET_ON);
  rotate = rotate_screen;
}
