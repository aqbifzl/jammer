#include "HardwareSerial.h"
#include "config.h"
#include "jammer.h"
#include "kbd.h"
#include "nrfmods.h"
#include "scanner.h"
#include "settings_page.h"
#include "state.h"
#include "terminal.h"
#include "tx_page.h"
#include "wifi_scan_page.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // wait for serial port to connect
  }

  delay(3000);

  Serial.println("nRF24L01 Channel Scanner and jammer");

  auto &kbd = Keyboard::instance();
  kbd.add_button(KEY_LEFT);
  kbd.add_button(KEY_RIGHT);
  kbd.add_button(KEY_SELECT);

  system_state_init();
  scanner_init();

  Terminal::instance().get_page()->draw();
  nrf_init();
  Jammer::instance().init_radios(&nrf1, &nrf2);
}

void loop() {
  auto &kbd = Keyboard::instance();

  kbd.update();

  InputEvent e;
  while (kbd.get_ev(&e)) {
    Terminal::instance().get_page()->handle_input(&e);
  }

  Terminal::instance().get_page()->update();
}
