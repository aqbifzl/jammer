#include "HardwareSerial.h"
#include "config.h"
#include "jammer.h"
#include "kbd.h"
#include "nrfmods.h"
#include "scanner_page.h"
#include "settings_page.h"
#include "state.h"
#include "terminal.h"
#include "tx_page.h"
#include "wifi_scan_page.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("nRF24L01 Channel Scanner and jammer");

  auto &kbd = Keyboard::instance();
  kbd.add_button(KEY_LEFT);
  kbd.add_button(KEY_RIGHT);
  kbd.add_button(KEY_SELECT);

  nrf_init();
  ScannerPage::instance()->init(&nrf1);
  Jammer::instance().init_radios(&nrf1, &nrf2);
  Terminal::instance().show_animation();
  Terminal::instance().get_page()->draw();
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
