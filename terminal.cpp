#include "terminal.h"
#include "config.h"
#include "scanner_page.h"
#include <Adafruit_GFX.h>
#define SSD1306_NO_SPLASH
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

  pages[(int)PageID::TX] = TXPage::instance();
  pages[(int)PageID::SCAN] = ScannerPage::instance();
  pages[(int)PageID::SETTINGS] = SettingsPage::instance();
  pages[(int)PageID::WIFI_SCAN] = WiFiPage::instance();
  current_page = PageID::TX;

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

Page *Terminal::get_page() const { return pages[(int)current_page]; }

Adafruit_SSD1306 *Terminal::get_display() { return &display; }

void Terminal::switch_page(PageID page) {
  int id_idx = (int)page;
  int current_idx = (int)current_page;

  if (page == current_page)
    return;
  if (id_idx < 0 || id_idx >= (int)PageID::MAX)
    return;

  if (pages[current_idx]) {
    pages[current_idx]->on_exit();
  }

  current_page = page;

  display.clearDisplay();

  if (pages[id_idx]) {
    pages[id_idx]->on_enter();
  }
}

void Terminal::next_page() {
  int next_idx = ((int)current_page + 1) % (int)PageID::MAX;
  switch_page((PageID)next_idx);
}

void Terminal::prev_page() {
  int prev_idx = (int)current_page - 1;
  if (prev_idx < 0) {
    prev_idx = (int)PageID::MAX - 1;
  }
  switch_page((PageID)prev_idx);
}

void Terminal::show_animation() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  const char *title = "JAMMER";

  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int title_x = (display.width() - w) / 2;
  int title_y = (display.height() / 2) - h;

  display.setCursor(title_x, title_y);
  display.print(title);

  display.setTextSize(1);
  const char *subtitle = "spektrum";

  display.getTextBounds(subtitle, 0, 0, &x1, &y1, &w, &h);
  int sub_x = (display.width() - w) / 2;
  int sub_y = title_y + 20;

  display.setCursor(sub_x, sub_y);
  display.print(subtitle);

  display.display();

  delay(1500);

  std::vector<int> active_pixels;
  active_pixels.reserve(display.width() * display.height() / 5);

  for (int y = 0; y < display.height(); y++) {
    for (int x = 0; x < display.width(); x++) {
      if (display.getPixel(x, y)) {
        active_pixels.push_back(y * display.width() + x);
      }
    }
  }

  int total_pixels = active_pixels.size();
  for (int i = total_pixels - 1; i > 0; --i) {
    int j = esp_random() % (i + 1);
    std::swap(active_pixels[i], active_pixels[j]);
  }

  int batches = 30;
  int pixels_per_batch = total_pixels / batches;
  if (pixels_per_batch < 1)
    pixels_per_batch = 1;

  for (int i = 0; i < total_pixels; ++i) {
    int idx = active_pixels[i];
    int x = idx % display.width();
    int y = idx / display.width();

    display.drawPixel(x, y, SSD1306_BLACK);

    if (i % pixels_per_batch == 0) {
      display.display();
      delay(10);
    }
  }

  display.clearDisplay();
  display.display();
  delay(200);
}
