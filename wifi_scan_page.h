#ifndef WIFI_PAGE_H
#define WIFI_PAGE_H

#include "menu_option.h"
#include "page.h"
#include "state.h"

#define FIRST_SCAN_MIN 20
#define FIRST_SCAN_MAX 20

#define SCAN_MIN 100
#define SCAN_MAX 200

struct WifiAP {
  char ssid[33]; // 32 chars + null term
  int32_t rssi;
  int32_t channel;
};

class WiFiPage : public Page {
private:
  WiFiPage();

  std::vector<WifiAP> access_points;
  int selected_index = 0;
  int scroll_offset = 0;
  bool scanning = false;

  TaskHandle_t scan_task_handle = nullptr;
  SemaphoreHandle_t data_mutex = nullptr;

  static void scan_task_entry(void *pvParameters);
  void scan_task_loop();

  bool inited = false;
  bool first_scan = true;
  bool init();

public:
  static WiFiPage *instance() {
    static WiFiPage sp;
    return &sp;
  }

  void on_enter() override;
  void on_exit() override;
  void draw() override;
  void handle_input(InputEvent *ev) override;
};

#endif
