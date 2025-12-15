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
const int zigbee_channels[] = {11, 15, 20, 25};
const int nrf_channels[] = {76, 78, 79};

const std::vector<Jammer::Mode> &Jammer::get_modes_vector() {
  static const std::vector<Mode> modes = {
      {"BLE", &Jammer::loop_ble},       {"WIFI", &Jammer::loop_wifi},
      {"BLE ADV", &Jammer::loop_blea},  {"BLE RND", &Jammer::loop_ble_rand},
      {"BT", &Jammer::loop_bt},         {"ALL", &Jammer::loop_all},
      {"ZIGBEE", &Jammer::loop_zigbee}, {"NRF", &Jammer::loop_nrf}};
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

  int power = system_state_get_tx_power();
  int speed = system_state_get_tx_speed();

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

  int mode_index = system_state_get_mode();
  if (mode_index < 0 && mode_index >= get_modes_vector().size()) {
    return;
  }
  mode_cache = mode_index;

  Serial.printf("starting a jammer (%s)\n",
                get_modes_vector()[mode_cache].name);

  constexpr int DEFAULT_CH = 45;

  setup_radio(_nrf1, DEFAULT_CH);
  setup_radio(_nrf2, DEFAULT_CH);

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

void Jammer::loop_bt() {
  for (int i = 2, j = 78; i <= j; i++, j--) {
    _nrf1->setChannel(i);
    _nrf2->setChannel(j);
  }
}

void Jammer::loop_blea() {
  int count =
      sizeof(ble_advertising_channels) / sizeof(ble_advertising_channels[0]);
  for (int i = 0, j = count - 1; i <= j; i++, j--) {
    _nrf1->setChannel(ble_advertising_channels[i]);
    _nrf2->setChannel(ble_advertising_channels[j]);
  }
}

void Jammer::loop_ble() {
  int count = sizeof(ble_data_channels) / sizeof(ble_data_channels[0]);
  for (int i = 0, j = count - 1; i <= j; i++, j--) {
    _nrf1->setChannel(ble_data_channels[i]);
    _nrf2->setChannel(ble_data_channels[j]);
  }
}

void Jammer::loop_ble_rand() {
  int count = sizeof(ble_data_channels) / sizeof(ble_data_channels[0]);

  int r1 = esp_random() % count;
  int r2 = esp_random() % count;

  _nrf1->setChannel(ble_data_channels[r1]);
  _nrf2->setChannel(ble_data_channels[r2]);
}

void Jammer::loop_all() {
  for (int i = 0, j = CHANNELS - 1; i <= j; i++, j--) {
    _nrf1->setChannel(i);
    _nrf2->setChannel(j);
  }
}

void Jammer::loop_zigbee() {
  int count = sizeof(zigbee_channels) / sizeof(zigbee_channels[0]);

  for (int i = 0, j = count - 1; i <= j; i++, j--) {
    _nrf1->setChannel(zigbee_channels[i]);
    _nrf2->setChannel(zigbee_channels[j]);
  }
}

void Jammer::loop_nrf() {
  int count = sizeof(nrf_channels) / sizeof(nrf_channels[0]);
  for (int i = 0, j = count - 1; i <= j; i++, j--) {
    _nrf1->setChannel(nrf_channels[i]);
    _nrf2->setChannel(nrf_channels[j]);
  }
}

void Jammer::loop_wifi() {
  for (int i = 1, j = 78; i <= j; i++, j--) {
    _nrf1->setChannel(i);
    _nrf2->setChannel(j);
  }
}
