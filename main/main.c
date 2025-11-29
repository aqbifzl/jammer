#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "jammer.h"
#include "kbd.h"
#include "scanner.h"
#include "state.h"
#include "terminal.h"

void app_main(void) {
  system_state_init();

  terminal_init();

  kbd_init();

  xTaskCreate(scanner_task, "scanner_task", 4096, NULL, 5, NULL);
  xTaskCreate(jammer_task, "jammer_task", 4096, NULL, 5, NULL);
}
