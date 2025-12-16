#include "jammer.h"
#include "HardwareSerial.h"
#include "config.h"
#include "nrfmods.h"
#include "sdkconfig.h"
#include "state.h"
#include "utils.h"
#include <Arduino.h>
#include <RF24.h>
#include <cstdint>
#include <functional>

int ble_advertising_channels[] = {2, 26, 80};
const int ble_data_channels[] = {
    4,  6,  8,  10, 12, 14, 16, 18, 20, 22, 24, 28, 30, 32, 34, 36, 38, 40, 42,
    44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78};
// const int zigbee_channels[] = {11, 15, 20, 25};
const int nrf_channels[] = {76, 78, 79};

const std::vector<Jammer::Mode> &Jammer::get_modes_vector() {
  static const std::vector<Mode> modes = {
      {"BLE", &Jammer::loop_ble},
      {"WIFI", &Jammer::loop_wifi},
      {"BLE ADV", &Jammer::loop_blea},
      {"BLE RND", &Jammer::loop_ble_rand},
      {"BT", &Jammer::loop_bt},
      {"ALL", &Jammer::loop_all},
      // {"ZIGBEE", &Jammer::loop_zigbee},
      {"NRF", &Jammer::loop_nrf},
      {"RANGE", &Jammer::loop_range},
  };
  return modes;
}

Jammer::Jammer() { _active = false; }

void Jammer::init_radios(RF24 *r1, RF24 *r2) {
  _nrf1 = r1;
  _nrf2 = r2;
}

void Jammer::setup_radio(RF24 *radio, uint8_t ch) {
  if (!radio)
    return;

  int power = SystemState::instance().global.tx_power;
  int speed = SystemState::instance().global.tx_speed;

  radio->powerUp();
  radio->setAutoAck(false);
  radio->stopListening();
  radio->setRetries(0, 0);
  radio->setPayloadSize(5);
  radio->setAddressWidth(3);
  radio->setCRCLength(RF24_CRC_DISABLED);

  radio->setPALevel(static_cast<rf24_pa_dbm_e>(power), true);
  radio->setDataRate(static_cast<rf24_datarate_e>(speed));

  radio->startConstCarrier(static_cast<rf24_pa_dbm_e>(power), ch);
}

void Jammer::start() {
  if (_active)
    return;

  int mode_index = SystemState::instance().tx_page.mode;
  if (mode_index < 0 && mode_index >= get_modes_vector().size()) {
    return;
  }
  mode_cache = mode_index;

  setup_radio(_nrf1, DEFAULT_CHANNEL);
  setup_radio(_nrf2, DEFAULT_CHANNEL);

  bool nrf1_works = _nrf1->isChipConnected();
  bool nrf2_works = _nrf2->isChipConnected();

  if (!nrf1_works || !nrf2_works) {
    Serial.println("one of the chips is disconnected, failed to start jamming");
    return;
  }

  Serial.printf("starting a jammer (%s)\n",
                get_modes_vector()[mode_cache].name);

  _active = true;
}

void Jammer::stop() {
  if (!_active)
    return;

  if (_nrf1)
    _nrf1->stopConstCarrier();
  if (_nrf2)
    _nrf2->stopConstCarrier();

  _active = false;

  Serial.println("jammer stopped");
}

void Jammer::update() {
  if (!_active || _nrf1 == nullptr || _nrf2 == nullptr)
    return;

  (this->*get_modes_vector()[mode_cache].loop_func)();
}

const char *Jammer::get_mode_name(int index) const {
  if (index >= 0 && index < get_modes_vector().size()) {
    return get_modes_vector()[index].name;
  }
  return "UNKNOWN";
}

int Jammer::get_mode_count() const { return get_modes_vector().size(); }

void Jammer::loop_bt() { run_range(3, 79, 2); }

void Jammer::loop_blea() {
  run_list(ble_advertising_channels, COUNT_OF(ble_advertising_channels));
}

void Jammer::loop_ble() {
  run_list(ble_data_channels, COUNT_OF(ble_data_channels));
}

void Jammer::loop_ble_rand() {
  constexpr int count = COUNT_OF(ble_data_channels);

  int r1 = esp_random() % count;
  int r2 = esp_random() % count;

  set_channels(ble_data_channels[r1], ble_data_channels[r2]);
}

void Jammer::loop_all() { run_range(0, CHANNELS - 1); }

// void Jammer::loop_zigbee() {
//   run_list(zigbee_channels, COUNT_OF(zigbee_channels));
// }

void Jammer::loop_nrf() { run_list(nrf_channels, COUNT_OF(nrf_channels)); }

void Jammer::loop_wifi() {
  // https://en.wikipedia.org/wiki/List_of_WLAN_channels
  const int wifi_ch = SystemState::instance().tx_page.wifi_channel;

  const uint8_t wifi_bw = SystemState::instance().tx_page.wifi_bandwidth;
  const uint8_t tx_bw =
      GlobalSettings::tx_speed_to_bw_mhz[SystemState::instance()
                                             .global.tx_speed];

  int center_nrf;
  if (wifi_ch >= 1 && wifi_ch <= 13) {
    center_nrf = 12 + (wifi_ch - 1) * 5;
  } else if (wifi_ch == 14) {
    center_nrf = 84;
  } else {
    return;
  }

  int half_width = wifi_bw / 2;

  int start = center_nrf - half_width;
  int end = center_nrf + half_width;

  run_range(center_nrf - 10, center_nrf + 10, tx_bw);
}

void Jammer::loop_range() {
  auto &settings = SystemState::instance().tx_page;

  run_range(settings.range_start, settings.range_end, settings.range_step);
}

inline void Jammer::set_channels(int ch1, int ch2) {
  // Serial.printf("c1=%d c2=%d\n", ch1, ch2);
  _nrf1->setChannel(ch1);
  _nrf2->setChannel(ch2);
}

void Jammer::run_range(int start, int end, int step) {
  if (step <= 0)
    step = 1;

  if (start < 0)
    start = 0;
  if (start > 125)
    start = 125;
  if (end < 0)
    end = 0;
  if (end > 125)
    end = 125;

  if (start > end)
    SWAP(start, end);

  for (int i = start, j = end; i <= j; i += step, j -= step) {
    set_channels(i, j);
  }
}

void Jammer::run_list(const int *list, size_t size) {
  if (size == 0)
    return;

  for (int i = 0, j = size - 1; i <= j; i++, j--) {
    set_channels(list[i], list[j]);
  }
}
