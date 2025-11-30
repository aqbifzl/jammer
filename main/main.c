#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "jammer.h"
#include "kbd.h"
#include "mirf.h"
#include "nrf24_device.h"
#include "scanner.h"
#include "state.h"
#include "terminal.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void app_main(void) {
  system_state_init();

  terminal_init();

  kbd_init();

  nrf24_init_device();

  if (!(jammer_event_group = xEventGroupCreate())) {
    printf("failed to create event group\n");
    return;
  }
  if (!(scanner_event_group = xEventGroupCreate())) {
    printf("failed to create event group\n");
    return;
  }

  xTaskCreate(jammer_task, "jammer_task", 4096, &nrf24_dev, 5,
              NULL);
  xTaskCreate(scanner_task, "scanner_task", 4096, &nrf24_dev, 5,
              NULL);

  scanner_init(&nrf24_dev);
  scanner_wait_for_start();

  vTaskDelay(portMAX_DELAY);
}
