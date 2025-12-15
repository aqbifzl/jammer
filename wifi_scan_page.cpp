#include "wifi_scan_page.h"
#include "config.h"
#include "esp_wifi_types_generic.h"
#include "terminal.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <cstring>
#include <esp_event.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

WiFiPage::WiFiPage() {}

bool WiFiPage::init() {
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

  return true;
}

void WiFiPage::scan_networks() {
  auto display = Terminal::instance().get_display();

  display->clearDisplay();
  display->setCursor(0, 0);
  display->println("Scanning WiFi...");
  display->display();

  wifi_scan_config_t scan_config = {
      .ssid = NULL,
      .bssid = NULL,
      .channel = 0,
      .show_hidden = true,
      .scan_type = WIFI_SCAN_TYPE_ACTIVE,
      .scan_time = {.active = {.min = 100, .max = 300}, .passive = 0}};
  esp_err_t err = esp_wifi_scan_start(&scan_config, true);

  access_points.clear();

  if (err == ESP_OK) {
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    if (ap_count > 0) {
      wifi_ap_record_t *ap_list =
          (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
      ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_list));

      for (int i = 0; i < ap_count; ++i) {
        WifiAP ap;
        snprintf(ap.ssid, 33, "%s", (char *)ap_list[i].ssid);

        if (strlen(ap.ssid) == 0) {
          snprintf(ap.ssid, 33, "<HIDDEN>");
        }

        ap.rssi = ap_list[i].rssi;
        ap.channel = ap_list[i].primary;
        access_points.push_back(ap);
      }
      free(ap_list);
    }
  } else {
    Serial.printf("Scan failed: %d\n", err);
  }

  selected_index = 0;
  scroll_offset = 0;
  needs_scan = false;
}

void WiFiPage::truncate_ssid_buffer(const char *source, char *dest,
                                    int max_len) {
  int len = strlen(source);

  if (len <= max_len) {
    strcpy(dest, source);
  } else {
    strncpy(dest, source, max_len);
    dest[max_len] = '.';
    dest[max_len + 1] = '.';
    dest[max_len + 2] = '.';
    dest[max_len + 3] = '\0';
  }
}

void WiFiPage::draw() {
  auto display = Terminal::instance().get_display();
  const int font_rows = Terminal::instance().get_font_rows();

  const int max_visible_items = (DISP_ROWS / font_rows) - 1;

  display->clearDisplay();
  display->setCursor(0, 0);

  display->setTextColor(SSD1306_WHITE);
  display->println("SSID      | CH | dBm");
  display->drawLine(0, font_rows - 1, 128, font_rows - 1, SSD1306_WHITE);

  if (access_points.empty()) {
    display->setCursor(0, font_rows + 2);
    display->println("No networks found.");
    display->println("Press SELECT to scan.");
  } else {
    char display_buffer[40];

    for (int i = 0; i < max_visible_items; ++i) {
      int data_idx = scroll_offset + i;
      if (data_idx >= access_points.size())
        break;

      WifiAP &ap = access_points[data_idx];
      int y_pos = (i + 1) * font_rows;

      if (data_idx == selected_index) {
        display->fillRect(0, y_pos, 128, font_rows, SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display->setTextColor(SSD1306_WHITE);
      }

      truncate_ssid_buffer(ap.ssid, display_buffer, 10);

      display->setCursor(0, y_pos);
      display->print(ap.ssid);

      int pad = 14 - strlen(ap.ssid);
      while (pad--)
        display->print(" ");

      display->print("| ");
      if (ap.channel < 10)
        display->print(" ");
      display->print(ap.channel);

      display->print(" | ");
      display->print(ap.rssi);
    }
  }

  display->display();
}

void WiFiPage::handle_input(InputEvent *ev) {
  if (handle_page_change(ev))
    return;

  // if (needs_scan)
  //   scan_networks();

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
      if (selected_index < access_points.size() - 1) {
        selected_index++;
        const int font_rows = Terminal::instance().get_font_rows();
        const int max_visible_items = (64 / font_rows) - 1;

        if (selected_index >= scroll_offset + max_visible_items) {
          scroll_offset++;
        }
      }
      break;
    case KEY_SELECT:
      scan_networks();
      // needs_scan = true;
      break;
    }
  }

  draw();
}
