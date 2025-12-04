#ifndef KBD_H
#define KBD_H

#include <Arduino.h>

enum kbd_pins_func {
  KEY_LEFT = GPIO_NUM_25,
  KEY_RIGHT = GPIO_NUM_26,
  KEY_SELECT = GPIO_NUM_27
};
typedef void (*kbd_callback_t)(int btn);
static const int KBD_PINS[] = {KEY_LEFT, KEY_RIGHT, KEY_SELECT};
#define KBD_COUNT (sizeof(KBD_PINS) / sizeof(KBD_PINS[0]))

void kbd_init();
void kbd_handle_input();
void kbd_set_short_press_callback(kbd_callback_t cb);
void kbd_set_long_repeat_callback(kbd_callback_t cb);

#endif // !KBD_H
