#include "nrf24_device.h"
#include "config.h" // For SPI_FREQ
#include "mirf.h"

NRF24_t nrf24_dev;

void nrf24_init_device(void) {
    Nrf24_set_spi_freq(SPI_FREQ);
    Nrf24_init(&nrf24_dev);
}
