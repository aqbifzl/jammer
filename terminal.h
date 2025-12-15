#ifndef TERMINAL_H
#define TERMINAL_H

#include "kbd.h"
#include "page.h"
#include <Adafruit_SSD1306.h>

class Terminal {
private:
  int _current_page;
  int font_cols, font_rows;
  Adafruit_SSD1306 display;
  Terminal();
  static const std::vector<Page *> pages;

public:
  static Terminal &instance() {
    static Terminal t;
    return t;
  }
  int get_font_cols() { return font_cols; }
  int get_font_rows() { return font_rows; }

  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;

  Adafruit_SSD1306 *get_display();
  Page *get_page() const;
  void next_page();
  void prev_page();
};

void draw_text_field(int16_t x, int16_t y, const char *text, bool highlight);

#endif // !TERMINAL_H
