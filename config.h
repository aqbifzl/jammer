#ifndef CONFIG_H
#define CONFIG_H

// general
#define DEFAULT_POWER 3
#define DEFAULT_CHANNEL 45
#define DEFAULT_SPEED 1

// nrf24l01
#define CHANNELS 126

// nrf1 is connected to vspi
#define NRF1_SPI VSPI
#define NRF1_MISO 19
#define NRF1_MOSI 23
#define NRF1_SCK 18
#define NRF1_CE 21
#define NRF1_CSN 22
// nrf2 is connected to hspi
#define NRF2_MISO 12
#define NRF2_MOSI 13
#define NRF2_SCK 14
#define NRF2_SPI HSPI
#define NRF2_CE 27
#define NRF2_CSN 26

// spectrum
#define INFO_ROWS (FONT_ROWS * 4)
#define MARGIN_TOP 5
#define SPECTRUM_X 0
#define SPECTRUM_Y (INFO_ROWS + MARGIN_TOP)

// display
#define DISP_COLS 128
#define DISP_ROWS 64

#define I2C_ADDRESS_1 0x3C
#define I2C_ADDRESS_2 0x3D

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 5

// keyboard
#define EVENT_QUEUE_SIZE 32
#define LONG_PRESS_START_MS 600
#define LONG_PRESS_REPEAT_MS 200
#define DEBOUNCE_MS 25

#define KEY_LEFT 32
#define KEY_RIGHT 33
#define KEY_SELECT 25

#endif // !CONFIG_H
