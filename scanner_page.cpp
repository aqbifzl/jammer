#include "scanner_page.h"
#include "HardwareSerial.h"
#include "RF24.h"
#include "config.h"
#include "kbd.h"
#include <Arduino.h>

int compare_stats(const void *a, const void *b) {
  TopChannelStat *statA = (TopChannelStat *)a;
  TopChannelStat *statB = (TopChannelStat *)b;
  return (int)(statB->count - statA->count);
}

void ScannerPage::init(RF24 *nrf) {
  if (inited)
    return;

  _nrf = nrf;
  inited = true;
}

void ScannerPage::scanner_scan() {
  for (int ch = 0; ch < CHANNELS; ch++) {
    _nrf->setChannel(ch);

    _nrf->startListening();
    delayMicroseconds(128);
    _nrf->stopListening();

    bool rpd_active = _nrf->testRPD();

    new_activity_state[ch] = rpd_active;
    if (rpd_active)
      ++stats[ch];
  }

  if (spectrum_changed(new_activity_state)) {
    set_spectrum_data_all(new_activity_state, CHANNELS);
  }
}

void ScannerPage::update_top_stats() {
  TopChannelStat temp_stats[CHANNELS];
  for (int i = 0; i < CHANNELS; i++) {
    temp_stats[i].channel = i;
    temp_stats[i].count = stats[i];
  }

  qsort(temp_stats, CHANNELS, sizeof(TopChannelStat), compare_stats);

  for (int i = 0; i < TOP_COUNT; i++) {
    top_stats[i] = temp_stats[i];
  }
}

void ScannerPage::draw() {
  auto *display = Terminal::instance().get_display();

  display->clearDisplay();

  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);

  int s_elapsed = (millis() - scan_start_time) / 1000;

  display->printf("%sTOP CHANNELS (%ds):\n", focused ? "[F] " : "", s_elapsed);

  String line = "";
  for (int i = 0; i < TOP_COUNT; i++) {
    if (top_stats[i].count == 0)
      continue;

    line += String(top_stats[i].channel);
    line += " ";
    line += String(top_stats[i].count);

    if (i < TOP_COUNT - 1 && top_stats[i + 1].count > 0) {
      line += "|";
    }
  }
  display->println(line);

  update_scroll_window();

  const int spikes = (SPECTRUM_W + GAP) / (SPIKE_W + GAP);

  int start = scroll_offset, end = start + spikes;
  if (end > CHANNELS)
    end = CHANNELS;

  for (int i = start; i < end; ++i) {
    int visible_index = i - start;

    bool active = spectrum_data[i];
    int spike_h = active ? SPECTRUM_H : 0;
    int spike_x = SPECTRUM_X + visible_index * (SPIKE_W + GAP);
    bool is_selected = (current_channel == i);

    // clear the column before drawing by drawing a black rectangle
    Terminal::instance().get_display()->fillRect(spike_x, SPECTRUM_Y, SPIKE_W,
                                                 SPECTRUM_H, SSD1306_BLACK);
    if (active) {
      display->fillRect(spike_x, SPECTRUM_Y + (SPECTRUM_H - spike_h), SPIKE_W,
                        spike_h, SSD1306_WHITE);
    } else {
      if (is_selected) {
        const int selected_box_height = 3;
        display->fillRect(spike_x,
                          SPECTRUM_Y + SPECTRUM_H - selected_box_height,
                          SPIKE_W, selected_box_height, SSD1306_WHITE);
      } else {
        int ly = SPECTRUM_Y + SPECTRUM_H - 1;
        display->drawLine(spike_x, ly, spike_x + SPIKE_W, ly, SSD1306_WHITE);
      }
    }
  }

  display->display();
}

void ScannerPage::update_scroll_window() {
  const int spikes = (DISP_COLS + GAP) / (SPIKE_W + GAP);

  if (current_channel < scroll_offset) {
    scroll_offset = current_channel;
  } else if (current_channel >= scroll_offset + spikes) {
    scroll_offset = current_channel - (spikes - 1);
  }

  const int max_offset = CHANNELS - spikes;
  if (scroll_offset > max_offset)
    scroll_offset = max_offset;
  if (scroll_offset < 0)
    scroll_offset = 0;
}

void ScannerPage::handle_input(InputEvent *ev) {
  if (!focused) {
    if (handle_page_change(ev))
      return;
  }

  if (ev->type == EVT_SHORT_PRESS && ev->pin == KEY_SELECT) {
    focused = !focused;
    return;
  }

  if (focused) {
    if (ev->type == EVT_LONG_PRESS_HOLD || ev->type == EVT_SHORT_PRESS) {
      if (ev->pin == KEY_LEFT) {
        if (current_channel > 0) {
          --current_channel;
        }
      }
      if (ev->pin == KEY_RIGHT) {
        if (current_channel < CHANNELS - 1) {
          ++current_channel;
        }
      }
      update_scroll_window();
    }
  }

  draw();
}

void ScannerPage::on_enter() {
  _nrf->powerDown();
  _nrf->powerUp();

  _nrf->setAutoAck(false);
  _nrf->setDataRate(RF24_1MBPS);
  _nrf->setCRCLength(RF24_CRC_DISABLED);
  _nrf->setPALevel(RF24_PA_HIGH);

  memset(top_stats, 0, sizeof(top_stats));
  memset(stats, 0, sizeof(stats));
  scan_start_time = millis();
  last_sort_time = 0;
}

void ScannerPage::update() {
  scanner_scan();
  unsigned long now = millis();

  if (now - last_sort_time > REFRESH_INTERVAL) {
    update_top_stats();
    last_sort_time = now;
  }

  if (now - scan_start_time > RESET_DATA_INTERVAL) {
    memset(stats, 0, sizeof(stats));
    memset(top_stats, 0, sizeof(top_stats));
    scan_start_time = now;
  }

  draw();
}

void ScannerPage::set_spectrum_data_all(const bool *data, size_t len) {
  if (!data) {
    return;
  }

  size_t count = (len < CHANNELS) ? len : CHANNELS;

  for (size_t i = 0; i < count; i++) {
    spectrum_data[i] = data[i];
  }
}

bool ScannerPage::spectrum_changed(bool *b) {
  return memcmp(b, spectrum_data, CHANNELS) != 0;
}
