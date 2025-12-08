#include "HardwareSerial.h"
#include "esp32-hal.h"
#include "freertos/idf_additions.h"
#include "jammer.h"
#include "kbd.h"
#include "nrfmods.h"
#include "scanner.h"
#include "state.h"
#include "terminal.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RF24.h>
#include <SPI.h>
#include <Wire.h>
#include <cstring>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>
#include <nRF24L01.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // wait for serial port to connect
  }

  delay(3000);

  Serial.println("nRF24L01 Channel Scanner and jammer");

  kbd_set_short_press_callback(system_state_on_short_press);
  kbd_set_long_repeat_callback(system_state_on_long_press_repeat);

  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_mem_release(ESP_BT_MODE_BTDM);

  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();

  system_state_init();
  nrf_init();
  scanner_init();
  kbd_init();
  terminal_init();
  jammer_init();
}

void loop() {
  kbd_handle_input();

  if (system_state_get_jamming())
    jammer_loop();
  else
    scanner_scan();
}
