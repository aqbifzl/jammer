#ifndef STATE_H
#define STATE_H

#include "config.h"
#include <cstddef>
#include <cstdint>

enum WiFiBandwidth : uint8_t { OFDM_20MHZ = 0, OFDM_40MHZ, DSSS_22MHZ, MAX };

struct GlobalSettings {
  // Set tx power : 0=-18dBm,1=-12dBm,2=-6dBm,3=0dBm
  static constexpr int8_t tx_power_to_dbm[] = {-18, -12, -6, 0};
  // 0=1Mbps, 1=2Mbps, 2=250Kbps
  static constexpr float tx_speed_to_mbps[] = {1, 2, 0.25};
  static constexpr uint8_t tx_speed_to_bw_mhz[] = {1, 2, 1};
  static constexpr uint8_t wifi_bandwidths[WiFiBandwidth::MAX] = {20, 22, 40};

  // 0=-18dBm,1=-12dBm,2=-6dBm,3=0dBm
  int tx_power = DEFAULT_POWER;
  // 0=1Mbps, 1=2Mbps, 2=250Kbps
  int tx_speed = DEFAULT_SPEED;
};

struct TxPageSettings {
  int mode = 0;
  // for wifi
  int wifi_channel = 1;
  int wifi_bandwidth = OFDM_20MHZ;
  // for range
  int range_start = 0;
  int range_end = 80;
  int range_step = 1;
};

class SystemState {
public:
  GlobalSettings global;
  TxPageSettings tx_page;

  static SystemState &instance() {
    static SystemState ss;
    return ss;
  }
};

#endif
