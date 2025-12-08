#include "kbd.h"
#include "HardwareSerial.h"
#include "config.h"
#include "esp32-hal-gpio.h"
#include <Arduino.h>

volatile bool irq_flags[KBD_COUNT] = {false};

static bool pressed[KBD_COUNT] = {false};
static bool long_started[KBD_COUNT] = {false};
static unsigned long press_start[KBD_COUNT] = {0};
static unsigned long last_change[KBD_COUNT] = {0};
static unsigned long last_repeat_time[KBD_COUNT] = {0};

static kbd_callback_t short_cb = nullptr;
static kbd_callback_t long_repeat_cb = nullptr;

// ISR for each button
void IRAM_ATTR handle_button0() { irq_flags[0] = true; }
void IRAM_ATTR handle_button1() { irq_flags[1] = true; }
void IRAM_ATTR handle_button2() { irq_flags[2] = true; }

typedef void (*ButtonISR)();
ButtonISR isrs[KBD_COUNT] = {handle_button0, handle_button1, handle_button2};

void kbd_init() {
  for (uint8_t i = 0; i < KBD_COUNT; i++) {
    pinMode(KBD_PINS[i], INPUT_PULLDOWN);
    attachInterrupt(KBD_PINS[i], isrs[i], CHANGE);
    irq_flags[i] = false;
    pressed[i] = false;
    long_started[i] = false;
    press_start[i] = 0;
    last_change[i] = 0;
    last_repeat_time[i] = 0;
  }
}

void kbd_handle_input() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < KBD_COUNT; ++i) {
    if (!irq_flags[i])
      continue;
    irq_flags[i] = false;

    bool level = digitalRead(KBD_PINS[i]);

    // debouncing
    if (now - last_change[i] < DEBOUNCE_MS) {
      continue;
    }

    last_change[i] = now;

    if (level == HIGH) {
      // pressed
      pressed[i] = true;
      long_started[i] = false;
      press_start[i] = now;
      last_repeat_time[i] = 0;
    } else {
      // released
      if (pressed[i]) {
        if (!long_started[i]) {
          if (short_cb) {
            short_cb(KBD_PINS[i]);
          }
        } else {
          // you can handle long press release here
        }
      }
      // reset button state
      pressed[i] = false;
      long_started[i] = false;
      press_start[i] = 0;
      last_repeat_time[i] = 0;
    }
  }

  for (uint8_t i = 0; i < KBD_COUNT; ++i) {
    if (!pressed[i])
      continue;

    unsigned long held = now - press_start[i];

    if (!long_started[i]) {
      if (held >= LONG_PRESS_START_MS) {
        long_started[i] = true;
        last_repeat_time[i] = now;
        // you can handle long press start here
      }
    } else {
      if (now - last_repeat_time[i] >= LONG_PRESS_REPEAT_MS) {
        last_repeat_time[i] = now;

        if (long_repeat_cb)
          long_repeat_cb(KBD_PINS[i]);
      }
    }
  }
}

void kbd_set_short_press_callback(kbd_callback_t cb) { short_cb = cb; }

void kbd_set_long_repeat_callback(kbd_callback_t cb) { long_repeat_cb = cb; }
