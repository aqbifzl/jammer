#include "jammer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void jammer_task(void *pvParameters) {
  while (1) {
    printf("jamming\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
