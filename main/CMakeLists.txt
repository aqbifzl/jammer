idf_component_register(SRCS main.c terminal.c kbd.c state.c
                       PRIV_REQUIRES spi_flash driver esp_timer
                       INCLUDE_DIRS ".")
