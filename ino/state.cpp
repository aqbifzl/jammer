#include "state.h"
#include "HardwareSerial.h"
#include "config.h"
#include "jammer.h"
#include "kbd.h"
#include "scanner.h"
#include "terminal.h"
#include <cstring>
#include <stdio.h>

static system_state_t state;

void system_state_init(void) {
  state.selected_ch = 0;
  state.tx_power = DEFAULT_POWER;
  state.tx_speed = DEFAULT_SPEED;
  state.highlighted = 0;
  state.selected = false;
  state.jamming = false;
  state.mode = MODE_SINGLE_CHANNEL;

  state.highlight_table[0] = &state.mode;
  state.highlight_table[1] = &state.selected_ch;
  state.highlight_table[2] = &state.tx_power;
  state.highlight_table[3] = &state.tx_speed;
  state.highlight_table[4] = &state.jamming;

  for (size_t i = 0; i < CHANNELS; i++) {
    state.spectrum_data[i] = false;
  }
}

int system_state_get_selected_ch(void) { return state.selected_ch; }

void system_state_set_selected_ch(int value) { state.selected_ch = value; }

int system_state_get_tx_power(void) { return state.tx_power; }

void system_state_set_tx_power(int value) { state.tx_power = value; }

int system_state_get_tx_speed(void) { return state.tx_speed; }

void system_state_set_tx_speed(int value) { state.tx_speed = value; }

bool system_state_get_spectrum_data(size_t index) {
  if (index >= CHANNELS) {
    return false;
  }
  return state.spectrum_data[index];
}

void system_state_set_spectrum_data(size_t index, bool value) {
  if (index >= CHANNELS) {
    return;
  }
  state.spectrum_data[index] = value;
}

void system_state_set_spectrum_data_all(const bool *data, size_t len) {
  if (!data) {
    return;
  }

  size_t count = (len < CHANNELS) ? len : CHANNELS;

  for (size_t i = 0; i < count; i++) {
    state.spectrum_data[i] = data[i];
  }
}

bool system_state_spectrum_changed(bool *b) {
  return memcmp(b, state.spectrum_data, CHANNELS) != 0;
}

int system_state_get_highlighted(void) { return state.highlighted; }

void system_state_set_highlighted(int value) { state.highlighted = value; }

bool system_state_get_selected(void) { return state.selected; }

void system_state_set_selected(bool value) { state.selected = value; }

void system_state_on_long_press_repeat(int pin) {
  if (state.selected)
    return;

  printf("LONG PRESS REPEAT pin=%d\n", pin);

  if (pin == KEY_LEFT && state.selected_ch > 0)
    --state.selected_ch;

  if (pin == KEY_RIGHT && state.selected_ch < CHANNELS - 1)
    ++state.selected_ch;

  terminal_redraw_all();
}

void system_state_on_short_press(int pin) {
  printf("SHORT PRESS pin=%d\n", pin);

  bool changed_something = false;

  switch (pin) {
  case KEY_LEFT: {
    if (state.selected) {
      int *opt_ptr = state.highlight_table[state.highlighted];
      if (opt_ptr && *opt_ptr > 0) {
        --(*opt_ptr);
        changed_something = true;
      }
    } else {
      if (state.highlighted > 0) {
        --state.highlighted;
        changed_something = true;
      }
    }

    break;
  }
  case KEY_RIGHT: {
    int *opt_ptr = state.highlight_table[state.highlighted];
    int max = max_option_sizes[state.highlighted];
    if (max <= 0) {
      break;
    }

    if (state.selected) {
      if (opt_ptr && *opt_ptr < (max - 1)) {
        ++(*opt_ptr);
        changed_something = true;
      }
    } else {
      if (state.highlighted < (OPT_MAX - 1)) {
        ++state.highlighted;
        changed_something = true;
      }
    }

    break;
  }
  case KEY_SELECT: {
    switch (state.highlighted) {
    case OPT_TX_BTN: {
      bool new_jamming_state = !system_state_get_jamming();
      system_state_set_jamming(new_jamming_state);

      if (new_jamming_state) {
        jammer_start(state.selected_ch);
      } else {
        jammer_stop();
        scanner_init();
      }

      Serial.println("starting/stopping jamming");
      break;
    }
    case OPT_MODE: {
      if (state.selected) {
        jammer_stop();
        state.jamming = false;
        switch (state.mode) {
        case MODE_SINGLE_CHANNEL:
          jammer_set_loop(nullptr);
          break;
        case MODE_BLE:
          jammer_set_loop(ble_loop);
          break;
        }
      }

      state.selected = !state.selected;
      break;
    }
    default:
      state.selected = !state.selected;
      break;
    }
  } break;
  default:
    break;
  }

  if (changed_something && state.highlighted == OPT_CHANNEL)
    terminal_redraw_spectrum();

  terminal_redraw_info();
}

bool system_state_get_jamming(void) { return state.jamming; }

void system_state_set_jamming(bool value) { state.jamming = value; }

int system_state_get_mode() { return state.mode; }
void system_state_set_mode(int mode) { state.mode = mode; }
const char *system_state_get_mode_str() { return MODE_STR[state.mode]; }
