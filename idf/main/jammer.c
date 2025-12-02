#include "jammer.h"
#include "config.h"
#include "esp_cpu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mirf.h"
#include "state.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

EventGroupHandle_t jammer_event_group;

void jammer_init(NRF24_t *dev) {
  Nrf24_config(dev, system_state_get_selected_ch(), MAX_RF_PAYLOAD);

  uint8_t zero = 0x00;

  Nrf24_writeRegister(dev, SETUP_RETR, &zero, 1); // disable retransmits
  Nrf24_writeRegister(dev, EN_AA, &zero, 1);      // disable auto-ack
  Nrf24_SetSpeedDataRates(dev, system_state_get_tx_speed()); // max data rate
  Nrf24_SetOutputRF_PWR(dev, system_state_get_tx_power());   // max power
  Nrf24_enableNoAckFeature(dev);

  uint8_t tx_address[5] = {'0', '0', '0', '0',
                           '1'}; // 5 bytes (no terminating 0)
  Nrf24_setTADDR(dev, tx_address);
}

void jammer_task(void *pvParameters) {
  NRF24_t *dev = (NRF24_t *)pvParameters;

  uint8_t payload[MAX_RF_PAYLOAD];
  memset(payload, 0xFF, COUNT_OF(payload));

  for (;;) {
    xEventGroupWaitBits(jammer_event_group, READY_BIT, pdFALSE, pdFALSE,
                        portMAX_DELAY);
    xEventGroupClearBits(jammer_event_group, STOPPED_BIT);

    while (xEventGroupGetBits(jammer_event_group) & READY_BIT) {
      Nrf24_sendNoAck(dev, payload);

      taskYIELD();
    }
    xEventGroupSetBits(jammer_event_group, STOPPED_BIT);
  }
}

void jammer_wait_for_start(void) {
  xEventGroupClearBits(jammer_event_group, STOPPED_BIT);
  xEventGroupSetBits(jammer_event_group, READY_BIT);
}

void jammer_wait_for_stop(void) {
  xEventGroupClearBits(jammer_event_group, READY_BIT);

  EventBits_t bits = xEventGroupWaitBits(jammer_event_group, STOPPED_BIT,
                                         pdTRUE, pdTRUE, pdMS_TO_TICKS(1000));

  if (!(bits & STOPPED_BIT)) {
    printf("warning: jammer didn't stop in time\n");
  }
}
