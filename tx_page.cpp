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

TXPage::TXPage() {
  highlighted = 0;
  selected = false;

  options.push_back({"MODE", 0, Jammer::instance().get_mode_count() - 1,
                     []() { return system_state_get_mode(); },
                     [](int v) { system_state_set_mode(v); },
                     [this](char *buf, int v) {
                       snprintf(buf, 32, "[%s]",
                                Jammer::instance().get_mode_name(v));
                     }});

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

void TXPage::draw() {
  auto display = Terminal::instance().get_display();
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

void TXPage::handle_input(InputEvent *ev) {
  if (ev->type == EVT_SHORT_PRESS && ev->pin == KEY_SELECT &&
      highlighted == 1) {
    if (Jammer::instance().is_active()) {
      Jammer::instance().stop();
    } else {
      Jammer::instance().start();
    }

    draw();
    return;
  }

  if (handle_basic_select(ev, options, highlighted, selected))
    return;

  draw();
}
