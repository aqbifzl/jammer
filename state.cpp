#include "state.h"
#include "HardwareSerial.h"
#include "config.h"
#include "esp32-hal-bt.h"
#include "jammer.h"
#include "kbd.h"
#include "scanner.h"
#include "settings_page.h"
#include "terminal.h"
#include <cstring>
#include <stdio.h>

static system_state_t state;

void system_state_init(void) {
  state.selected_ch = DEFAULT_CHANNEL;
  state.tx_power = DEFAULT_POWER;
  state.tx_speed = DEFAULT_SPEED;
  state.mode = 0;

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

int system_state_get_mode() { return state.mode; }
void system_state_set_mode(int mode) { state.mode = mode; }
