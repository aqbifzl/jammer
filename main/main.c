#include "config.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "kbd.h"
#include "mirf.h"
#include "state.h"
#include "terminal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void app_main(void) {
  system_state_init();

  terminal_init();

  Nrf24_set_spi_freq(SPI_FREQ);
  NRF24_t dev;
  Nrf24_init(&dev);

  uint8_t rpd;
  bool active[CHANNELS] = {0};
  bool prev_active[CHANNELS] = {0};

  kbd_init();

  terminal_redraw();

  while (1) {
    for (int ch = 0; ch < CHANNELS; ++ch) {
      Nrf24_configRegister(&dev, RF_CH, ch);

      // it's power up function without clearing IRQ, we don't use IRQ and it
      // saves about 150 microseconds
      dev.PTX = 0;
      Nrf24_ceLow(&dev);
      Nrf24_configRegister(
          &dev, CONFIG,
          mirf_CONFIG |
              ((1 << PWR_UP) | (1 << PRIM_RX))); // set device as RX mode
      Nrf24_ceHi(&dev);

      esp_rom_delay_us(WAIT_AFTER_INIT_US);

      Nrf24_readRegister(&dev, RPD, &rpd, 1);
      if (rpd & 1) {
        active[ch] = 1;
      }
    }

    if (memcmp(active, prev_active, sizeof(active)) != 0) {
      system_state_set_spectrum_data_all(active, CHANNELS);

      terminal_redraw();
      memcpy(prev_active, active, sizeof(active));
    }
    memset(active, 0, sizeof(active));

    // we should consider adding timeout but right now i don't see any problem
    // with scanning as fast as possible
  }

  Nrf24_deinit(&dev);
}
