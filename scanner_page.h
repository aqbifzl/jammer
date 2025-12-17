#ifndef SCANNER_PAGE_H
#define SCANNER_PAGE_H

#include "RF24.h"
#include "menu_option.h"
#include "page.h"
#include "state.h"
#include <cstdint>

struct TopChannelStat {
  uint8_t channel;
  uint32_t count;
};

class ScannerPage : public Page {
private:
  RF24 *_nrf = nullptr;
  int current_channel = 0;
  bool spectrum_data[CHANNELS] = {0};
  bool focused = false;
  bool new_activity_state[CHANNELS] = {0};
  int scroll_offset = 0;
  bool inited = false;
  uint32_t stats[CHANNELS] = {0};

  uint64_t scan_start_time;
  uint64_t last_sort_time;
  static const int TOP_COUNT = 10;
  static const int REFRESH_INTERVAL = 1000;
  static const int RESET_DATA_INTERVAL = 30000;
  TopChannelStat top_stats[TOP_COUNT] = {0};

  static void scan_task_entry(void *pvParameters);
  void update() override;
  void scan_task_loop();
  void scan_next();
  void scanner_scan();
  void set_spectrum_data_all(const bool *data, size_t len);
  bool spectrum_changed(bool *b);
  void update_scroll_window();
  void update_top_stats();

public:
  static ScannerPage *instance() {
    static ScannerPage sp;
    return &sp;
  }

  void init(RF24 *_nrf);
  void on_enter() override;
  void draw() override;
  void handle_input(InputEvent *ev) override;
};

#endif
