#include "wifi_scan_page.h"
#include "HardwareSerial.h"
#include "config.h"
#include "esp_err.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "terminal.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include <esp_event.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

WiFiPage::WiFiPage() {
  scanning = false;
  scan_task_handle = nullptr;
  first_scan = true;
  data_mutex = xSemaphoreCreateMutex();
}

bool WiFiPage::init() {
  if (inited)
    return true;

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  if (esp_netif_init() != ESP_OK) {
    return false;
  }

  if (esp_event_loop_create_default() != ESP_OK) {
    return false;
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  err = esp_wifi_init(&cfg);

  if (err != ESP_ERR_INVALID_STATE) {
    ESP_ERROR_CHECK(err);
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  inited = true;
  return true;
}

void WiFiPage::draw() {
  auto display = Terminal::instance().get_display();
  const int font_rows = Terminal::instance().get_font_rows();

  const int max_visible_items = (DISP_ROWS / font_rows) - 1;

  display->clearDisplay();
  display->setCursor(0, 0);

  display->setTextColor(SSD1306_WHITE);
  display->printf("SSID    | CH | dBm%s\n", scanning ? "(S)" : "");
  display->drawLine(0, font_rows - 1, 128, font_rows - 1, SSD1306_WHITE);

  char wifi_entry_buffer[32 + 2 + 3];

  if (access_points.empty()) {
    display->setCursor(0, font_rows + 2);
    display->println("No networks found.");

  } else {
    for (int i = 0; i < max_visible_items; ++i) {
      int data_idx = scroll_offset + i;
      if (data_idx >= access_points.size())
        break;

      WifiAP &ap = access_points[data_idx];
      int y_pos = (i + 1) * font_rows + 1;

      if (data_idx == selected_index) {
        display->fillRect(0, y_pos, 128, font_rows, SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display->setTextColor(SSD1306_WHITE);
      }

      display->setTextWrap(false);
      display->setCursor(0, y_pos);

      const int total_chars = DISP_COLS / Terminal::instance().get_font_cols();

      // rssi - 3 chars
      // channel - 2 chars
      // 2 separators
      const int fixed_chars = 2 + 3 + 2;

      int ssid_len = total_chars - fixed_chars;
      if (ssid_len < 1)
        ssid_len = 1;

      snprintf(wifi_entry_buffer, sizeof(wifi_entry_buffer), "%-*.*s|%2d|%d",
               ssid_len, ssid_len, ap.ssid, ap.channel, ap.rssi);

      display->print(wifi_entry_buffer);
      display->setTextWrap(true);
    }
  }

  display->display();
}

void WiFiPage::handle_input(InputEvent *ev) {
  if (handle_page_change(ev))
    return;

  if (ev->type == EVT_SHORT_PRESS) {
    size_t ap_size = 0;
    if (xSemaphoreTake(data_mutex, 10)) {
      ap_size = access_points.size();
      xSemaphoreGive(data_mutex);
    }

    if (ev->type == EVT_SHORT_PRESS) {
      switch (ev->pin) {
      case KEY_LEFT:
        if (selected_index > 0) {
          selected_index--;
          if (selected_index < scroll_offset) {
            scroll_offset = selected_index;
          }
        }
        break;
      case KEY_RIGHT:
        if (selected_index < ap_size - 1) {
          selected_index++;
          const int font_rows = Terminal::instance().get_font_rows();
          const int max_visible_items = (64 / font_rows) - 1;

          if (selected_index >= scroll_offset + max_visible_items) {
            scroll_offset++;
          }
        }
        break;
      }
    }
  }

  draw();
}

void WiFiPage::on_enter() {
  scanning = false;

  if (!inited)
    init();

  if (scan_task_handle == nullptr) {
    xTaskCreate(scan_task_entry, "wifi_scan_task", 4096, this, 1,
                &scan_task_handle);
  }
}

void WiFiPage::on_exit() {
  if (scan_task_handle != nullptr) {
    vTaskDelete(scan_task_handle);
    scan_task_handle = nullptr;
    scanning = false;
  }
}

void WiFiPage::scan_task_loop() {
  const TickType_t xDelay = pdMS_TO_TICKS(5000);

  while (true) {
    this->scanning = true;
    draw();

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {.min = (first_scan ? FIRST_SCAN_MIN : SCAN_MIN),
                       .max = (first_scan ? FIRST_SCAN_MAX : SCAN_MAX)},
            .passive = 0}};

    esp_err_t err = esp_wifi_scan_start(&scan_config, true);

    if (err == ESP_OK) {
      uint16_t ap_count = 0;
      esp_wifi_scan_get_ap_num(&ap_count);

      if (ap_count > 0) {
        wifi_ap_record_t *ap_list =
            (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
        esp_wifi_scan_get_ap_records(&ap_count, ap_list);

        if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
          access_points.clear();

          for (int i = 0; i < ap_count; ++i) {
            WifiAP ap;
            snprintf(ap.ssid, 33, "%s", (char *)ap_list[i].ssid);
            if (strlen(ap.ssid) == 0)
              snprintf(ap.ssid, 33, "<HIDDEN>");
            ap.rssi = ap_list[i].rssi;
            ap.channel = ap_list[i].primary;

            access_points.push_back(ap);
          }

          xSemaphoreGive(data_mutex);
        }

        free(ap_list);
      }
    } else {
      Serial.printf("Scan failed: %d\n", err);
    }

    if (first_scan)
      first_scan = false;

    this->scanning = false;
    draw();

    vTaskDelay(xDelay);
  }
}

void WiFiPage::scan_task_entry(void *pvParameters) {
  WiFiPage *page = static_cast<WiFiPage *>(pvParameters);
  page->scan_task_loop();
  vTaskDelete(nullptr);
}
