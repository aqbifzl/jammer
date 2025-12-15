#include "terminal.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "HardwareSerial.h"
#include "settings_page.h"
#include "tx_page.h"
#include "wifi_scan_page.h"
#include <Fonts/FreeMono9pt7b.h>
#include <cstdint>
#include <cstdio>
#include <stdbool.h>
#include <stdint.h>

const std::vector<Page *> Terminal::pages = {
    TXPage::instance(), SettingsPage::instance(), WifiPage::instance()};

void Terminal::next_page() {
  _current_page = (_current_page + 1) % pages.size();
}

void Terminal::prev_page() {
  int prev_index = _current_page - 1;

  if (prev_index < 0) {
    prev_index = pages.size() - 1;
  }

  _current_page = prev_index;
}

Terminal::Terminal() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  display = Adafruit_SSD1306(DISP_COLS, DISP_ROWS, &Wire);

  bool ok = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS_1);
  if (!ok) {
    ok = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS_2);
    if (!ok) {
      Serial.println(F("display initialization failed"));
      while (1)
        ;
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

  display.getTextBounds("a", 0, 0, &dummy, &dummy, &w, &h);

  font_cols = w, font_rows = h;

  Serial.printf("w,h = %d %d", w, h);
}

void draw_text_field(int16_t x, int16_t y, const char *text, bool highlight) {
  auto display = Terminal::instance().get_display();

  int16_t bx, by;
  uint16_t bw, bh;

  display->getTextBounds(text, x, y, &bx, &by, &bw, &bh);

  if (highlight) {
    display->fillRect(bx, by, bw, bh, SSD1306_WHITE);
    display->setTextColor(SSD1306_BLACK);
  } else {
    display->setTextColor(SSD1306_WHITE);
  }

  display->setCursor(x, y);
  display->print(text);

  display->setTextColor(SSD1306_WHITE);
}

Page *Terminal::get_page() const { return pages[_current_page]; }

Adafruit_SSD1306 *Terminal::get_display() { return &display; }
