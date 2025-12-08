#ifndef JAMMER_H
#define JAMMER_H

#include <stdint.h>
typedef void (*jammer_loop_t)(void);

void jammer_init();
void jammer_start(uint8_t ch);
void jammer_stop();
void jammer_set_loop(jammer_loop_t loop);
void jammer_loop();

void ble_loop();
void blea_loop();
void bt_loop();
void all_loop();
void zigbee_loop();
void nrf_loop();
void ble_rand_loop();

#endif // JAMMER_H
