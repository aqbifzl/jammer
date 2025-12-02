#include "state.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "jammer.h"
#include "kbd.h"
#include "nrf24_device.h"
#include "scanner.h"
#include "terminal.h"
#include <stdio.h>

static system_state_t state;

void system_state_init(void) {
  state.selected_ch = 0;
  state.tx_power = 0;
  state.tx_speed = 0;
  state.highlighted = 0;
  state.selected = false;
  state.jamming = false;

  state.highlight_table[0] = &state.selected_ch;
  state.highlight_table[1] = &state.tx_power;
  state.highlight_table[2] = &state.tx_speed;
  state.highlight_table[3] = &state.jamming;

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

int system_state_get_highlighted(void) { return state.highlighted; }

void system_state_set_highlighted(int value) { state.highlighted = value; }

bool system_state_get_selected(void) { return state.selected; }

void system_state_set_selected(bool value) { state.selected = value; }

void system_state_on_long_press_start(int pin) {
  printf("LONG PRESS START pin=%d\n", pin);
}

void system_state_on_long_press_repeat(int pin) {
  printf("LONG PRESS REPEAT pin=%d\n", pin);
  if (state.selected)
    return;

  if (pin == KEY_LEFT && state.selected_ch > 0)
    --state.selected_ch;

  if (pin == KEY_RIGHT && state.selected_ch < CHANNELS - 1)
    ++state.selected_ch;

  terminal_redraw_spectrum();
}

void system_state_on_short_press(int pin) {
  printf("SHORT PRESS pin=%d\n", pin);
  switch (pin) {
  case KEY_LEFT: {
    if (state.selected && *state.highlight_table[state.highlighted] > 0) {
      *state.highlight_table[state.highlighted] -= 1;
    } else if (state.highlighted > 0) {
      --state.highlighted;
    }

    break;
  }
  case KEY_RIGHT: {
    int max = max_option_sizes[state.highlighted];
    if (state.selected && *state.highlight_table[state.highlighted] < max - 1) {
      *state.highlight_table[state.highlighted] += 1;
    } else if (state.highlighted < OPT_MAX - 1) {
      ++state.highlighted;
    }

    break;
  }
  case KEY_SELECT: {
    if (state.highlighted != OPT_TX_BTN) {
      state.selected = !state.selected;
      break;
    }

    bool new_jamming_state = !system_state_get_jamming();
    system_state_set_jamming(new_jamming_state);

    if (new_jamming_state) {
      scanner_wait_for_stop();
      jammer_wait_for_stop();

      jammer_init(&nrf24_dev);
      jammer_wait_for_start();
    } else {
      jammer_wait_for_start();
      jammer_wait_for_stop();

      scanner_init(&nrf24_dev);
      scanner_wait_for_start();
    }
  } break;
  }

  terminal_redraw_info();
}

void system_state_on_long_release(int pin) {
  printf("LONG PRESS END pin=%d\n", pin);
}

bool system_state_get_jamming(void) { return state.jamming; }

void system_state_set_jamming(bool value) { state.jamming = value; }
