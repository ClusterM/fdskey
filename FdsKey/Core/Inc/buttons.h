#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

uint8_t button_up_holding();
uint8_t button_down_holding();
uint8_t button_left_holding();
uint8_t button_right_holding();
uint8_t button_up_newpress();
uint8_t button_down_newpress();
uint8_t button_left_newpress();
uint8_t button_right_newpress();
void button_left_right_repeat_enable(uint8_t enable);
uint32_t button_left_hold_time();
uint32_t button_right_hold_time();
void button_check_screen_off();

#define BUTTONS_REPEAT_TIME 500
#define BUTTONS_LEFT_RIGHT_REPEAT_INTERVAL 12

#endif /* INC_BUTTONS_H_ */
