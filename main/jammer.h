#ifndef JAMMER_H
#define JAMMER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mirf.h"

extern EventGroupHandle_t jammer_event_group;

void jammer_init(NRF24_t* dev);
void jammer_task(void *pvParameters);
void jammer_wait_for_start(void);
void jammer_wait_for_stop(void);

#endif // JAMMER_H
