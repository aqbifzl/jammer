#ifndef KBD_H
#define KBD_H

#include "soc/gpio_num.h"

enum kbd_pins_func {
  KEY_LEFT = GPIO_NUM_25,
  KEY_RIGHT = GPIO_NUM_26,
  KEY_SELECT = GPIO_NUM_27
};
static const int kbd_pins[] = {KEY_LEFT, KEY_RIGHT, KEY_SELECT};
#define KBD_COUNT (sizeof(kbd_pins) / sizeof(kbd_pins[0]))

void kbd_init();

#endif // !KBD_H
