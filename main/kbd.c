#include "kbd.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "state.h"
#include "terminal.h"
#include <stdint.h>

typedef enum { BUTTON_EVENT_DOWN, BUTTON_EVENT_UP } button_event_type_t;

#define QUEUE_LEN 16
#define LONG_PRESS_US (2 * 1000000)
#define REPEAT_PERIOD_US 100000
#define DEBOUNCE_US 5000

typedef struct {
  button_event_type_t type;
  gpio_num_t pin;
  int64_t timestamp_us;
} button_event_t;

static QueueHandle_t button_queue;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  uintptr_t pin_u = (uintptr_t)arg;
  gpio_num_t pin = (gpio_num_t)pin_u;

  int level = gpio_get_level(pin);

  button_event_t evt;
  evt.type = (level == 1) ? BUTTON_EVENT_DOWN : BUTTON_EVENT_UP;
  evt.pin = pin;
  evt.timestamp_us = esp_timer_get_time();

  BaseType_t woken = pdFALSE;
  xQueueSendFromISR(button_queue, &evt, &woken);
  if (woken)
    portYIELD_FROM_ISR();
}

static int pin_to_index(gpio_num_t pin) {
  for (size_t i = 0; i < KBD_COUNT; ++i) {
    if (kbd_pins[i] == pin)
      return (int)i;
  }
  return -1;
}

typedef struct {
  int64_t last_evt_ts;
  int64_t down_ts;
  int64_t last_repeat_ts;
  bool is_down;
  bool long_active;
} button_state_t;

static button_state_t btn_state[KBD_COUNT];

void longpress_task(void *arg) {
  (void)arg;

  for (;;) {
    int64_t now = esp_timer_get_time();

    for (int i = 0; i < KBD_COUNT; ++i) {
      button_state_t *st = &btn_state[i];

      if (!st->is_down)
        continue;

      int64_t held_us = now - st->down_ts;

      if (!st->long_active && held_us >= LONG_PRESS_US) {
        st->long_active = true;
        st->last_repeat_ts = now;
        system_state_on_long_press_start(kbd_pins[i]);
      }

      if (st->long_active && (now - st->last_repeat_ts) >= REPEAT_PERIOD_US) {
        st->last_repeat_ts = now;
        system_state_on_long_press_repeat(kbd_pins[i]);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void button_task(void *arg) {
  (void)arg;
  button_event_t evt;

  for (;;) {
    if (!xQueueReceive(button_queue, &evt, portMAX_DELAY))
      continue;

    int idx = pin_to_index(evt.pin);
    if (idx < 0)
      continue;

    int64_t now = evt.timestamp_us;
    button_state_t *st = &btn_state[idx];

    if (now - st->last_evt_ts < DEBOUNCE_US)
      continue;

    st->last_evt_ts = now;

    if (evt.type == BUTTON_EVENT_DOWN) {
      if (st->is_down)
        continue;

      st->is_down = true;
      st->down_ts = now;
      st->long_active = false;
      st->last_repeat_ts = 0;
      continue;
    }

    if (evt.type == BUTTON_EVENT_UP) {
      if (!st->is_down)
        continue;

      st->is_down = false;

      int64_t press_dur = now - st->down_ts;

      if (!st->long_active && press_dur < LONG_PRESS_US)
        system_state_on_short_press(evt.pin);
      else
        system_state_on_long_release(evt.pin);

      st->long_active = false;
      st->last_repeat_ts = 0;
      continue;
    }
  }
}

void init_buttons() {
  button_queue = xQueueCreate(QUEUE_LEN, sizeof(button_event_t));
  if (!button_queue) {
    printf("button queue create failed\n");
    return;
  }

  gpio_install_isr_service(0);

  for (size_t i = 0; i < KBD_COUNT; ++i) {
    gpio_num_t pin = kbd_pins[i];
    gpio_config_t cfg = {.pin_bit_mask = 1ULL << pin,
                         .mode = GPIO_MODE_INPUT,
                         .pull_up_en = GPIO_PULLUP_DISABLE,
                         .pull_down_en = GPIO_PULLDOWN_ENABLE,
                         .intr_type = GPIO_INTR_ANYEDGE};
    gpio_config(&cfg);

    gpio_isr_handler_add(pin, gpio_isr_handler, (void *)(uintptr_t)pin);
  }
}

void kbd_init() {
  init_buttons();

  xTaskCreate(button_task, "button_task", 2048, NULL, tskIDLE_PRIORITY + 2,
              NULL);

  xTaskCreate(longpress_task, "longpress_task", 2048, NULL,
              tskIDLE_PRIORITY + 1, NULL);
}
