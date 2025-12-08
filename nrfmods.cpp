#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>

#include "config.h"
#include "esp32-hal-spi.h"
#include "nrfmods.h"

#define SPI_FREQ 16000000

SPIClass vspi(NRF1_SPI);
RF24 nrf1(NRF1_CS, NRF1_CSN, SPI_FREQ);

SPIClass hspi(NRF2_SPI);
RF24 nrf2(NRF2_CS, NRF2_CSN, SPI_FREQ);

void nrf_init() {
  vspi.begin(NRF1_SCK, NRF1_MISO, NRF1_MOSI);

  if (!nrf1.begin(&vspi)) {
    Serial.println("failed to init nrf module 1");
    while (1) {
    }
  }

  hspi.begin(NRF2_SCK, NRF2_MISO, NRF2_MOSI);

  if (!nrf2.begin(&hspi)) {
    Serial.println("failed to init nrf module 2");
    while (1) {
    }
  }
}
