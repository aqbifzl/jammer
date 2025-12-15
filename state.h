#ifndef STATE_H
#define STATE_H
#include "config.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>

// Set tx power : 0=-18dBm,1=-12dBm,2=-6dBm,3=0dBm
static const int tx_power_to_dbm[] = {-18, -12, -6, 0};

// 0=1Mbps, 1=2Mbps, 2=250Kbps
static const float tx_speed_to_mbps[] = {1, 2, 0.25};

static const int tx_speed_to_bw_mhz[] = {1, 2, 1};

typedef struct {
  // Selected channel, used for transmission 0-125
  int selected_ch;
  // 0=-18dBm,1=-12dBm,2=-6dBm,3=0dBm
  int tx_power;
  // 0=1Mbps, 1=2Mbps, 2=250Kbps
  int tx_speed;
  // true if there's some activity on channel
  bool spectrum_data[CHANNELS];
  int mode;
} system_state_t;

void system_state_init();

int system_state_get_selected_ch();
void system_state_set_selected_ch(int value);

int system_state_get_tx_power();
void system_state_set_tx_power(int value);

int system_state_get_tx_speed();
void system_state_set_tx_speed(int value);

bool system_state_get_spectrum_data(size_t index);
void system_state_set_spectrum_data(size_t index, bool value);
void system_state_set_spectrum_data_all(const bool *data, size_t len);
bool system_state_spectrum_changed(bool *b);

int system_state_get_mode();
void system_state_set_mode(int mode);

#endif
