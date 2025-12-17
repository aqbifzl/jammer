#ifndef TERMINAL_H
#define TERMINAL_H

#include "kbd.h"
#include "page.h"
#define SSD1306_NO_SPLASH
#include <Adafruit_SSD1306.h>
#include <cstdint>

enum class PageID : uint8_t { TX = 0, SCAN, SETTINGS, WIFI_SCAN, MAX };

class Terminal {
private:
  Page *pages[(int)PageID::MAX];
  PageID current_page;

  int font_cols, font_rows;
  Adafruit_SSD1306 display;
  Terminal();

public:
  static Terminal &instance() {
    static Terminal t;
    return t;
  }
  int get_font_cols() { return font_cols; }
  int get_font_rows() { return font_rows; }
  void show_animation();

  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;

  Adafruit_SSD1306 *get_display();
  Page *get_page() const;
  void next_page();
  void prev_page();
  void switch_page(PageID page);
};

void draw_text_field(int16_t x, int16_t y, const char *text, bool highlight);

#endif // !TERMINAL_H
