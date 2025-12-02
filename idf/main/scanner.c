#include "scanner.h"
#include "config.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "mirf.h"
#include "state.h"
#include "terminal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

EventGroupHandle_t scanner_event_group;

void scanner_init(NRF24_t *dev) {}

void scanner_wait_for_start(void) {
  xEventGroupClearBits(scanner_event_group, STOPPED_BIT);
  xEventGroupSetBits(scanner_event_group, READY_BIT);
}

void scanner_wait_for_stop(void) {

  xEventGroupClearBits(scanner_event_group, READY_BIT);

  EventBits_t bits = xEventGroupWaitBits(scanner_event_group, STOPPED_BIT,
                                         pdTRUE, pdTRUE, pdMS_TO_TICKS(1000));

  if (!(bits & STOPPED_BIT)) {
    printf("warning: scanner didn't stop in time\n");
  }
}

void scanner_task(void *pvParameters) {
  NRF24_t *dev = (NRF24_t *)pvParameters;

  uint8_t rpd;
  bool active[CHANNELS] = {0};
  bool prev_active[CHANNELS] = {0};

  while (1) {
    xEventGroupWaitBits(scanner_event_group, READY_BIT, pdFALSE, pdFALSE,
                        portMAX_DELAY);
    xEventGroupClearBits(scanner_event_group, STOPPED_BIT);

    while (xEventGroupGetBits(scanner_event_group) & READY_BIT) {
      for (int ch = 0; ch < CHANNELS; ++ch) {
        Nrf24_configRegister(dev, RF_CH, ch);

        // it's power up function without clearing IRQ, we don't use IRQ and it
        // saves about 150 microseconds
        dev->PTX = 0;
        Nrf24_ceLow(dev);
        Nrf24_configRegister(
            dev, CONFIG,
            mirf_CONFIG |
                ((1 << PWR_UP) | (1 << PRIM_RX))); // set device as RX mode
        Nrf24_ceHi(dev);

        esp_rom_delay_us(WAIT_AFTER_INIT_US);

        Nrf24_readRegister(dev, RPD, &rpd, 1);
        if (rpd & 1) {
          active[ch] = 1;
        }
      }

      if (memcmp(active, prev_active, sizeof(active)) != 0) {
        system_state_set_spectrum_data_all(active, CHANNELS);
        terminal_redraw_spectrum();
        memcpy(prev_active, active, sizeof(active));
      }
      memset(active, 0, sizeof(active));

      taskYIELD();
    }
    xEventGroupSetBits(scanner_event_group, STOPPED_BIT);
  }
}
