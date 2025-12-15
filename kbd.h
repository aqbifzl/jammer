#ifndef KBD_H
#define KBD_H

#include "config.h"
#include <Arduino.h>
#include <cstdint>
#include <vector>

enum EventType { EVT_SHORT_PRESS, EVT_LONG_PRESS_START, EVT_LONG_PRESS_HOLD };

struct InputEvent {
  EventType type;
  uint8_t pin;
  uint32_t timestamp;
};

class Keyboard {
private:
  struct ButtonState {
    uint8_t pin;
    bool pressed;
    bool long_pressed;
    uint64_t last_repeat;

    volatile bool debouncing;
    volatile uint64_t debouncing_start_time;
  };

  volatile InputEvent eventQueue[EVENT_QUEUE_SIZE];
  volatile uint8_t head = 0;
  volatile uint8_t tail = 0;

  std::vector<ButtonState *> buttons;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  static void IRAM_ATTR isrHandler(void *arg);
  void push_to_queue(InputEvent evt);

public:
  static Keyboard &instance() {
    static Keyboard s;
    return s;
  }

  void add_button(uint8_t pin);
  void update();
  bool get_ev(InputEvent *ev);
};

#endif // !KBD_H
