#include "scanner.h"
#include "HardwareSerial.h"
#include "config.h"
#include "nrfmods.h"
#include "state.h"
#include "terminal.h"
#include <Arduino.h>
#include <cstring>
#include <stdbool.h>

static bool new_activity_state[CHANNELS] = {0};

void scanner_init() {
  nrf1.powerDown();
  nrf1.powerUp();

  nrf1.setAutoAck(false);
  nrf1.setDataRate(RF24_1MBPS);
  nrf1.setCRCLength(RF24_CRC_DISABLED);
  nrf1.setPALevel(RF24_PA_HIGH);
}

static int ch = 0;

// currently unused but may be useful for optimisation, duplicate on purpose
void scan_next() {
  nrf1.setChannel(ch);

  nrf1.startListening();
  delayMicroseconds(128);
  nrf1.stopListening();

  new_activity_state[ch] = nrf1.testRPD();
  ch++;
  if (ch >= CHANNELS) {
    ch = 0;
  }
}

void scanner_scan() {
  for (ch = 0; ch < CHANNELS; ch++) {
    nrf1.setChannel(ch);

    nrf1.startListening();
    delayMicroseconds(128);
    nrf1.stopListening();

    new_activity_state[ch] = nrf1.testRPD();
  }

  if (SystemState::instance().scanner.spectrum_changed(new_activity_state)) {
    SystemState::instance().scanner.set_spectrum_data_all(new_activity_state,
                                                          CHANNELS);
  }
}
