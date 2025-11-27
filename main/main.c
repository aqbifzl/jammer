#include "config.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "kbd.h"
#include "mirf.h"
#include "state.h"
#include "terminal.h"
#include <stdint.h>
#include <stdio.h>

void app_main(void) {
  system_state_init();
  terminal_init();
  redraw();

  Nrf24_set_spi_freq(SPI_FREQ);
  NRF24_t dev;
  Nrf24_init(&dev);

  uint8_t rpd;
  uint8_t active[CHANNELS] = {0};

  kbd_init();
  while (1) {
    uint64_t scan_start_us = esp_timer_get_time();
    uint64_t elapsed_us = 0;

    while (elapsed_us < UPDATE_INTERVAL_US) {
      for (int ch = 0; ch < CHANNELS; ++ch) {
        Nrf24_configRegister(&dev, RF_CH, ch);
        Nrf24_powerUpRx(&dev);

        esp_rom_delay_us(WAIT_AFTER_INIT_US);

        Nrf24_readRegister(&dev, RPD, &rpd, 1);
        if (rpd & 1) {
          active[ch] = 1;
        }
      }

      elapsed_us = esp_timer_get_time() - scan_start_us;
    }

    // printf("active frequencies: ");
    // for (int ch = 0; ch < CHANNELS; ++ch) {
    //   if (active[ch]) {
    //     printf("%d ", 2400 + ch + 1);
    //     active[ch] = 0;
    //   }
    // }
    // printf("MHz\n");
  }

  Nrf24_deinit(&dev);
}
