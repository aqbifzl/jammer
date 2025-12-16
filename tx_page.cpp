#include "tx_page.h"
#include "HardwareSerial.h"
#include "config.h"
#include "freertos/idf_additions.h"
#include "jammer.h"
#include "kbd.h"
#include "state.h"
#include "terminal.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <cstring>

TXPage::TXPage() {
  highlighted = 0;
  selected = false;

  build_options();
}

void TXPage::draw() {
  auto display = Terminal::instance().get_display();
  const int font_rows = Terminal::instance().get_font_rows();

  display->clearDisplay();

  // display->fillRect(0, 0, DISP_COLS, info_rows, SSD1306_BLACK);

  char val_buffer[32];
  char line_buffer[64];
  // char buffer[64];

  for (int i = 0; i < options.size(); ++i) {
    int y_pos = i * font_rows;
    int current_val = options[i].get_val();

    options[i].format_text(val_buffer, current_val);

    if (strcmp(options[i].name, "ACTION") == 0) {
      snprintf(line_buffer, sizeof(line_buffer), "%s", val_buffer);
    } else {
      snprintf(line_buffer, sizeof(line_buffer), "%-6s %s", options[i].name,
               val_buffer);
    }

    draw_text_field(0, y_pos, line_buffer, i == highlighted);
  }

  Terminal::instance().get_display()->display();
}

void TXPage::handle_input(InputEvent *ev) {
  if (ev->type == EVT_SHORT_PRESS && ev->pin == KEY_SELECT &&
      strcmp(options[highlighted].name, "ACTION") == 0) {
    if (Jammer::instance().is_active()) {
      Jammer::instance().stop();
    } else {
      Jammer::instance().start();
    }

    draw();
    return;
  }

  int old_mode = SystemState::instance().tx_page.mode;

  if (handle_basic_select(ev, options, highlighted, selected))
    return;

  int new_mode = SystemState::instance().tx_page.mode;
  if (old_mode != new_mode) {
    build_options();

    if (highlighted >= options.size()) {
      highlighted = options.size() - 1;
    }
  }

  draw();
}

void TXPage::update() { Jammer::instance().update(); }

void TXPage::build_options() {
  options.clear();

  options.push_back({"MODE", 0, Jammer::instance().get_mode_count() - 1,
                     []() { return SystemState::instance().tx_page.mode; },
                     [](int v) { SystemState::instance().tx_page.mode = v; },
                     [this](char *buf, int v) {
                       snprintf(buf, 32, "[%s]",
                                Jammer::instance().get_mode_name(v));
                     }});

  int current_mode_idx = SystemState::instance().tx_page.mode;
  const char *mode_name = Jammer::instance().get_mode_name(current_mode_idx);

  if (strcmp(mode_name, "WIFI") == 0) {
    options.push_back(
        {"CH", 1, 14,
         []() { return SystemState::instance().tx_page.wifi_channel; },
         [](int v) { SystemState::instance().tx_page.wifi_channel = v; },
         [](char *buf, int v) { snprintf(buf, 32, "%d", v); }});

    options.push_back(
        {"BW", 0, WiFiBandwidth::MAX,
         []() { return SystemState::instance().tx_page.wifi_bandwidth; },
         [](int v) { SystemState::instance().tx_page.wifi_bandwidth = v; },
         [](char *buf, int v) {
           snprintf(
               buf, 32, "%dMHz",
               GlobalSettings::wifi_bandwidths[SystemState::instance()
                                                   .tx_page.wifi_bandwidth]);
         }});
  }

  else if (strcmp(mode_name, "RANGE") == 0) {
    options.push_back(
        {"START", 0, 125,
         []() { return SystemState::instance().tx_page.range_start; },
         [](int v) { SystemState::instance().tx_page.range_start = v; },
         [](char *buf, int v) { snprintf(buf, 32, "%d", v); }});

    options.push_back(
        {"END", 0, 125,
         []() { return SystemState::instance().tx_page.range_end; },
         [](int v) { SystemState::instance().tx_page.range_end = v; },
         [](char *buf, int v) { snprintf(buf, 32, "%d", v); }});

    options.push_back(
        {"STEP", 1, 20,
         []() { return SystemState::instance().tx_page.range_step; },
         [](int v) { SystemState::instance().tx_page.range_step = v; },
         [](char *buf, int v) { snprintf(buf, 32, "%d", v); }});
  }

  options.push_back({"ACTION", 0, 0,
                     []() { return Jammer::instance().is_active(); },
                     [](int v) {},
                     [](char *buf, int v) {
                       if (v == 1) {
                         snprintf(buf, 32, "[STOP]");
                       } else {
                         snprintf(buf, 32, "[START]");
                       }
                     }});
}
