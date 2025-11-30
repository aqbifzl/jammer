#ifndef MAIN_NRF24_DEVICE_H_
#define MAIN_NRF24_DEVICE_H_

#include "mirf.h"

#ifdef __cplusplus
extern "C" {
#endif

extern NRF24_t nrf24_dev;

void nrf24_init_device(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_NRF24_DEVICE_H_ */
