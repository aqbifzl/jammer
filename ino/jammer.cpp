#include "jammer.h"
#include "HardwareSerial.h"
#include "RF24.h"
#include "config.h"
#include "nrfmods.h"
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

void ble_loop() {
  for (int i = 1, j = 79; i <= j; i += 2, j -= 2) {
    nrf1.setChannel(i);
    nrf2.setChannel(j);
  }
}

void jammer_loop() {
  if (loop_func) {
    loop_func();
  }
}
