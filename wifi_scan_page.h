#ifndef WIFI_PAGE_H
#define WIFI_PAGE_H

#include "menu_option.h"
#include "page.h"
#include "state.h"

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
  bool needs_scan = true;

  void truncate_ssid_buffer(const char *source, char *dest, int max_len);

  void scan_networks();

public:
  static WiFiPage *instance() {
    static WiFiPage sp;
    return &sp;
  }
  bool init();
  void on_enter() override { needs_scan = true; }
  void draw() override;
  void handle_input(InputEvent *ev) override;
};

#endif
