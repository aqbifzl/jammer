#include "jammer.h"
#include "HardwareSerial.h"
#include "RF24.h"
#include "config.h"
#include "nrfmods.h"
#include "sdkconfig.h"
#include "state.h"
#include <Arduino.h>
#include <RF24.h>
#include <cstdint>

static jammer_loop_t loop_func = nullptr;

void jammer_init() {}

void jammer_init(RF24 *mod, uint8_t ch) {
  Serial.printf("starting jammer on %d\n", ch);
  // stopConstCarrier calls power down
  mod->powerUp();

  mod->setAutoAck(false);
  mod->stopListening();
  mod->setRetries(0, 0);
  mod->setPayloadSize(5);
  mod->setAddressWidth(3);
  mod->setCRCLength(RF24_CRC_DISABLED);

  mod->setPALevel((rf24_pa_dbm_e)system_state_get_tx_power(), true);
  mod->setDataRate((rf24_datarate_e)system_state_get_tx_speed());
  mod->startConstCarrier((rf24_pa_dbm_e)system_state_get_tx_power(), ch);

  mod->printPrettyDetails();
}

void jammer_start(uint8_t ch) {
  jammer_init(&nrf1, ch);
  jammer_init(&nrf2, ch);
}

void jammer_stop() {
  nrf1.stopConstCarrier();
  nrf2.stopConstCarrier();
}

void jammer_set_loop(jammer_loop_t loop) { loop_func = loop; }

void bt_loop() {
  for (int i = 2, j = 78; i <= j; i += 1, j -= 1) {
    nrf1.setChannel(i);
    nrf2.setChannel(j);
  }
}

int ble_advertising_channels[] = {2, 26, 80};

void blea_loop() {
  for (int i = 0, j = COUNT_OF(ble_advertising_channels) - 1; i <= j;
       i += 1, j -= 1) {
    nrf1.setChannel(ble_advertising_channels[i]);
    nrf2.setChannel(ble_advertising_channels[j]);
  }
}

const int ble_data_channels[] = {
    4,  6,  8,  10, 12, 14, 16, 18, 20, 22, 24, 28, 30, 32, 34, 36, 38, 40, 42,
    44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78};

void ble_loop() {
  for (int i = 0, j = COUNT_OF(ble_data_channels) - 1; i <= j; i += 1, j -= 1) {
    nrf1.setChannel(ble_data_channels[i]);
    nrf2.setChannel(ble_data_channels[j]);
  }
}

void all_loop() {
  for (int i = 0, j = CHANNELS - 1; i <= i; i += 1, j -= 1) {
    nrf1.setChannel(ble_data_channels[i]);
    nrf2.setChannel(ble_data_channels[j]);
  }
}

const int zigbee_channels[] = {11, 15, 20, 25};

void zigbee_loop() {
  for (int i = 0, j = COUNT_OF(zigbee_channels) - 1; i <= j; i += 1, j -= 1) {
    nrf1.setChannel(zigbee_channels[i]);
    nrf2.setChannel(zigbee_channels[j]);
  }
}

const int nrf_channels[] = {76, 78, 79};

void nrf_loop() {
  for (int i = 0, j = COUNT_OF(nrf_channels) - 1; i <= j; i += 1, j -= 1) {
    nrf1.setChannel(nrf_channels[i]);
    nrf2.setChannel(nrf_channels[j]);
  }
}

void jammer_loop() {
  if (loop_func) {
    loop_func();
  }
}
