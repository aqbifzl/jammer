#include "terminal.h"
#include "config.h"
#include "state.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "HardwareSerial.h"
#include <Fonts/FreeMono9pt7b.h>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SPIKE_W 2
#define GAP 1

#define DISP_ROWS 64
#define DISP_COLS 128

#define COL_CHARS (DISP_COLS / FONT_COLS)

#define LEFT_COL 0
#define RIGHT_COL (DISP_COLS / 2)

#define MAX_BUF ((DISP_COLS / FONT_COLS) + 1)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

static int scroll_offset = 0;
static int FONT_COLS, FONT_ROWS;

void update_scroll_window(void) {
  const int spikes = (DISP_COLS + GAP) / (SPIKE_W + GAP);

  const int selected_ch = system_state_get_selected_ch();

  if (selected_ch < scroll_offset) {
    scroll_offset = selected_ch;
  } else if (selected_ch >= scroll_offset + spikes) {
    scroll_offset = selected_ch - (spikes - 1);
  }

  const int max_offset = CHANNELS - spikes;
  if (scroll_offset > max_offset)
    scroll_offset = max_offset;
  if (scroll_offset < 0)
    scroll_offset = 0;
}

void terminal_redraw_spectrum() {
  const int x = SPECTRUM_X;
  const int y = SPECTRUM_Y;
  const int w = DISP_COLS - x;
  const int h = DISP_ROWS - y;

  update_scroll_window();

  const int spikes = (w + GAP) / (SPIKE_W + GAP);

  int start = scroll_offset, end = start + spikes;
  if (end > CHANNELS)
    end = CHANNELS;

  const int selected_ch = system_state_get_selected_ch();
  for (int i = start; i < end; ++i) {
    int visible_index = i - start;

    bool active = system_state_get_spectrum_data(i);
    int spike_h = active ? h : 0;
    int spike_x = x + visible_index * (SPIKE_W + GAP);
    bool is_selected = (selected_ch == i);

    // clear the column before drawing by drawing a black rectangle
    display.fillRect(spike_x, y, SPIKE_W, h, SSD1306_BLACK);
    if (active) {
      display.fillRect(spike_x, y + (h - spike_h), SPIKE_W, spike_h,
                       SSD1306_WHITE);
    } else {
      if (is_selected) {
        const int selected_box_height = 3;
        display.fillRect(spike_x, y + h - selected_box_height, SPIKE_W,
                         selected_box_height, SSD1306_WHITE);
      } else {
        int ly = y + h - 1;
        display.drawLine(spike_x, ly, spike_x + SPIKE_W, ly, SSD1306_WHITE);
      }
    }
  }

  display.display();
}

// 0 fail, 1 = addr 1, 2 = addr 2
int try_init_display() {
  bool ok = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS_1);
  if (!ok) {
    ok = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS_2);
    return ok ? 2 : 0;
  }
  return 1;
}

void terminal_init() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  if (!try_init_display()) {
    Serial.println("display initialization failed");
    while (1) {
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();

  int16_t dummy;
  uint16_t w, h;

  const char dummy_str = 'a';

  display.getTextBounds("a", 0, 0, &dummy, &dummy, &w, &h);

  FONT_COLS = w, FONT_ROWS = h;

  Serial.printf("w,h = %d %d", w, h);

  terminal_redraw_all();
}

void draw_text_field(int16_t x, int16_t y, const char *text, bool highlight) {
  int16_t bx, by;
  uint16_t bw, bh;

  display.getTextBounds(text, x, y, &bx, &by, &bw, &bh);

  if (highlight) {
    display.fillRect(bx, by, bw, bh, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(SSD1306_WHITE);
  }

  display.setCursor(x, y);
  display.print(text);

  display.setTextColor(SSD1306_WHITE);
}

void terminal_redraw_info() {
  display.fillRect(0, 0, DISP_COLS, INFO_ROWS, SSD1306_BLACK);

  char buffers[OPT_MAX][MAX_BUF];
  int lengths[OPT_MAX];

  const int selected_ch = system_state_get_selected_ch();
  const int tx_power = system_state_get_tx_power();
  const int tx_speed = system_state_get_tx_speed();

  const int hl = system_state_get_highlighted();

  char buffer[int(SCREEN_WIDTH / FONT_COLS)];

  sprintf(buffer, "[%s]", system_state_get_mode_str());
  draw_text_field(0, 0, buffer, hl == OPT_MODE);
  display.print(" ");

  sprintf(buffer, "C=%d", selected_ch);
  draw_text_field(display.getCursorX(), 0, buffer, hl == OPT_CHANNEL);
  display.print(" ");
  sprintf(buffer, "P=%ddBm", tx_power_to_dbm[tx_power]);
  draw_text_field(display.getCursorX(), 0, buffer, hl == OPT_POWER);

  snprintf(buffer, sizeof(buffer), "S=%.2fMbps(%dMHz)",
           tx_speed_to_mbps[tx_speed], tx_speed_to_bw_mhz[tx_speed]);
  draw_text_field(0, FONT_ROWS, buffer, hl == OPT_SPEED);

  draw_text_field(0, FONT_ROWS * 2,
                  system_state_get_jamming() ? "[STOP]" : "[START]",
                  hl == OPT_TX_BTN);

  display.display();
}

void terminal_redraw_all() {
  terminal_redraw_info();
  terminal_redraw_spectrum();
}
