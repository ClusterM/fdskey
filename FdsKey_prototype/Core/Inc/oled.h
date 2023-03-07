#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"
#include "fonts.h"
#include "images.h"

#define OLED_I2C hi2c1
#define OLED_ADDRESS 0x3C

#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_PADDING_LEFT 0

#define OLED_TIMEOUT 100

#define OLED_COMMAND_LAST 0x00
#define OLED_COMMAND_NOT_LAST 0x80
#define OLED_COMMAND_DATA 0x40

#define OLED_CMD_SET_COLUMN_LOW(column) ((column) & 0xF)
#define OLED_CMD_SET_COLUMN_HIGH(column) (0x10 | ((column) >> 8))
#define OLED_CMD_SET_PUMP_VOLTAGE_6_4 0x30
#define OLED_CMD_SET_PUMP_VOLTAGE_7_4 0x31
#define OLED_CMD_SET_PUMP_VOLTAGE_8_0 0x32
#define OLED_CMD_SET_PUMP_VOLTAGE_9_0 0x33
#define OLED_CMD_SET_START_LINE(line) (0x40 | ((line) & 0x3F))
#define OLED_CMD_SET_CONTRAST_MODE 0x81
#define OLED_CMD_SET_HORIZONTAL_FLIP_OFF 0xA0
#define OLED_CMD_SET_HORIZONTAL_FLIP_ON 0xA1
#define OLED_CMD_SET_ENTIRE_OFF 0xA4
#define OLED_CMD_SET_ENTIRE_ON 0xA5
#define OLED_CMD_SET_REVERSE_OFF 0xA6
#define OLED_CMD_SET_REVERSE_ON 0xA7
#define OLED_CMD_SET_MULTIPLEX_RATION_MODE 0xA8
#define OLED_CMD_SET_OFF 0xAE
#define OLED_CMD_SET_ON 0xAF
#define OLED_CMD_SET_PAGE(page) (0xB0 | ((page) & 7))
#define OLED_CMD_SET_VERTICAL_FLIP_OFF 0xC0
#define OLED_CMD_SET_VERTICAL_FLIP_ON 0xC8
#define OLED_CMD_SET_CLOCK_RATIO_MODE 0xD5
#define OLED_CMD_SET_PADS_MODE 0xDA
#define OLED_CMD_SET_PADS_MODE_SEQUENTIAL 0x02
#define OLED_CMD_SET_PADS_MODE_ALTERNATIVE 0x12

void oled_init(uint8_t rotate_screen, uint8_t reverse, uint8_t contrast);
uint8_t* oled_pixel(int x, int y);
void oled_set_pixel(int x, int y, uint8_t value);
uint8_t oled_get_pixel(int x, int y);
HAL_StatusTypeDef oled_send_commands(int len, ...);
HAL_StatusTypeDef oled_send_command(uint8_t command);
HAL_StatusTypeDef oled_write_data(uint8_t *data, uint8_t len);
HAL_StatusTypeDef oled_update(uint8_t start_page, uint8_t end_page);
HAL_StatusTypeDef oled_update_full();
HAL_StatusTypeDef oled_update_invisible();
HAL_StatusTypeDef oled_set_line(int y);
uint8_t oled_get_line();

void oled_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fill, uint8_t value);
void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t value);
void oled_copy_to_invisible();
void oled_switch_to_invisible();
void oled_draw_text(const DotMatrixFont *font, char* text, int x, int y, uint8_t replace, uint8_t invert);
void oled_draw_text_cropped(const DotMatrixFont *font, char* text, int x, int y,
    uint8_t start_x, uint8_t max_width,
    uint8_t start_y, uint8_t max_height,
    uint8_t replace, uint8_t invert);
void oled_draw_image(const DotMatrixImage *img, int x, int y, uint8_t replace, uint8_t invert);
void oled_draw_image_cropped(const DotMatrixImage *img, int x, int y,
    uint8_t start_x, uint8_t max_width,
    uint8_t start_y, uint8_t max_height,
    uint8_t replace, uint8_t invert);

extern I2C_HandleTypeDef OLED_I2C;

#endif /* INC_OLED_H_ */
