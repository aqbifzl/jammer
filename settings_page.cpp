#include "settings_page.h"
#include "config.h"
#include "freertos/idf_additions.h"
#include "jammer.h"
#include "kbd.h"
#include "state.h"
#include "terminal.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

SettingsPage::SettingsPage() {
  highlighted = 0;
  selected = false;

  options.push_back({"TX POWER", 0, COUNT_OF(tx_power_to_dbm) - 1,
                     []() { return system_state_get_tx_power(); },
                     [](int v) { system_state_set_tx_power(v); },
                     [](char *buf, int v) {
                       snprintf(buf, 32, "TX POWER: %ddBm", tx_power_to_dbm[v]);
                     }});

  options.push_back({"SPEED", 0, COUNT_OF(tx_speed_to_bw_mhz) - 1,
                     []() { return system_state_get_tx_speed(); },
                     [](int v) { system_state_set_tx_speed(v); },
                     [](char *buf, int v) {
                       snprintf(buf, 32, "SPEED=%.2fMbps(%dMHz)",
                                tx_speed_to_mbps[v], tx_speed_to_bw_mhz[v]);
                     }});
}

void SettingsPage::draw() {
  auto display = Terminal::instance().get_display();
  const int font_cols = Terminal::instance().get_font_cols();
  const int font_rows = Terminal::instance().get_font_rows();
  const int info_rows = options.size() * font_rows;

  display->fillRect(0, 0, DISP_COLS, info_rows, SSD1306_BLACK);

  char buffer[64];

  for (int i = 0; i < options.size(); ++i) {
    int y_pos = i * font_rows;
    int current_val = options[i].get_val();

    options[i].format_text(buffer, current_val);

    draw_text_field(0, y_pos, buffer, i == highlighted);
  }

  Terminal::instance().get_display()->display();
}

void SettingsPage::handle_input(InputEvent *ev) {
  if (handle_basic_select(ev, options, highlighted, selected))
    return;

  draw();
}
