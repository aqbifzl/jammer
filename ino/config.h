#ifndef CONFIG_H
#define CONFIG_H

// general
#define DEFAULT_POWER 3 
#define DEFAULT_SPEED 1

// nrf24l01
#define CHANNELS 126

// nrf1 is connected to vspi
#define NRF1_SPI VSPI
#define NRF1_MISO 19
#define NRF1_MOSI 23
#define NRF1_SCK  18
#define NRF1_CS 16
#define NRF1_CSN 17
// nrf2 is connected to hspi
#define NRF2_MISO 12
#define NRF2_MOSI 13
#define NRF2_SCK  14
#define NRF2_SPI HSPI
#define NRF2_CS 4
#define NRF2_CSN 2

// spectrum
#define INFO_ROWS (FONT_ROWS * 4)
#define MARGIN_TOP 5
#define SPECTRUM_X 0
#define SPECTRUM_Y (INFO_ROWS + MARGIN_TOP)

// display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_ADDRESS_1 0x3C
#define I2C_ADDRESS_2 0x3D

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// keyboard
#define LONG_PRESS_START_MS                                                    \
  1000UL
#define LONG_PRESS_REPEAT_MS 100UL
#define DEBOUNCE_MS 40UL

#endif // !CONFIG_H
