#ifndef SCANNER_H
#define SCANNER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mirf.h"

extern EventGroupHandle_t scanner_event_group;

void scanner_task(void *pvParameters);
void scanner_init(NRF24_t* dev);
void scanner_wait_for_start(void);
void scanner_wait_for_stop(void);

#endif // SCANNER_H
