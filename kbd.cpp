#include "kbd.h"
#include "config.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"

void IRAM_ATTR Keyboard::isrHandler(void *arg) {
  ButtonState *btn = static_cast<ButtonState *>(arg);

  detachInterrupt(btn->pin);

  btn->debouncing = true;
  btn->debouncing_start_time = millis();
}

void Keyboard::push_to_queue(InputEvent evt) {
  uint8_t nextHead = (head + 1) % EVENT_QUEUE_SIZE;
  if (nextHead != tail) {
    event_queue[head].type = evt.type;
    event_queue[head].pin = evt.pin;
    event_queue[head].timestamp = evt.timestamp;
    head = nextHead;
  }
}

void Keyboard::add_button(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
  bool pressed_now = !digitalRead(pin);

  ButtonState *btn = new ButtonState{pin, pressed_now, false, 0, false, 0};
  buttons.push_back(btn);

  attachInterruptArg(digitalPinToInterrupt(pin), isrHandler, btn, CHANGE);
}

void Keyboard::update() {
  unsigned long now = millis();

  for (auto btn : buttons) {
    if (btn->debouncing) {
      if (now - btn->debouncing_start_time >= DEBOUNCE_MS) {
        bool state = !digitalRead(btn->pin); // pull-up

        if (state != btn->pressed) {
          btn->pressed = state;

          if (state) {
            btn->long_pressed = false;
            btn->last_repeat = now;
          } else {
            if (!btn->long_pressed) {
              portENTER_CRITICAL(&mux);
              push_to_queue({EVT_SHORT_PRESS, btn->pin, (uint32_t)now});
              portEXIT_CRITICAL(&mux);
            }
            btn->long_pressed = false;
          }
        }

        btn->debouncing = false;
        attachInterruptArg(digitalPinToInterrupt(btn->pin), isrHandler, btn,
                           CHANGE);
      }
    }

    if (btn->pressed && !btn->debouncing) {

      unsigned long duration = now - btn->debouncing_start_time;

      if (!btn->long_pressed && duration >= LONG_PRESS_START_MS) {
        btn->long_pressed = true;
        btn->last_repeat = now;

        portENTER_CRITICAL(&mux);
        push_to_queue({EVT_LONG_PRESS_START, btn->pin, (uint32_t)now});
        portEXIT_CRITICAL(&mux);
      }

      if (btn->long_pressed &&
          (now - btn->last_repeat >= LONG_PRESS_REPEAT_MS)) {
        btn->last_repeat = now;

        portENTER_CRITICAL(&mux);
        push_to_queue({EVT_LONG_PRESS_HOLD, btn->pin, (uint32_t)now});
        portEXIT_CRITICAL(&mux);
      }
    }
  }
}

bool Keyboard::get_ev(InputEvent *ev) {
  if (head == tail)
    return false;

  portENTER_CRITICAL(&mux);
  ev->type = event_queue[tail].type;
  ev->pin = event_queue[tail].pin;
  ev->timestamp = event_queue[tail].timestamp;

  tail = (tail + 1) % EVENT_QUEUE_SIZE;
  portEXIT_CRITICAL(&mux);

  return true;
}
