#ifndef MENU_OPTION_H
#define MENU_OPTION_H

#include "HardwareSerial.h"
#include "config.h"
#include "kbd.h"
#include "terminal.h"
#include <functional>

struct MenuOption {
  const char *name;
  int min_val;
  int max_val;

  std::function<int()> get_val;
  std::function<void(int)> set_val;
  std::function<void(char *buffer, int value)> format_text;
};

// true when page is changed
static bool handle_page_change(InputEvent *ev) {
  if (ev->type == EVT_LONG_PRESS_START) {
    if (ev->pin == KEY_SELECT)
      return false;

    if (ev->pin == KEY_LEFT)
      Terminal::instance().prev_page();
    if (ev->pin == KEY_RIGHT)
      Terminal::instance().next_page();

    return true;
  }

  return false;
}

// true when page is changed
static bool handle_basic_select(InputEvent *ev,
                                std::vector<MenuOption> &options,
                                int &highlighted, bool &selected) {
  if (handle_page_change(ev))
    return true;

  if (ev->type != EVT_SHORT_PRESS)
    return false;

  MenuOption &opt = options[highlighted];
  int current_val = opt.get_val();

  switch (ev->pin) {
  case KEY_LEFT:
    if (selected) {
      if (current_val > opt.min_val) {
        opt.set_val(current_val - 1);
      } else {
        opt.set_val(opt.max_val);
      }
    } else {
      if (highlighted > 0)
        highlighted--;
    }
    break;

  case KEY_RIGHT:
    if (selected) {
      if (current_val < opt.max_val) {
        opt.set_val(current_val + 1);
      } else {
        opt.set_val(opt.min_val);
      }
    } else {
      if (highlighted < options.size() - 1)
        highlighted++;
    }
    break;

  case KEY_SELECT:
    selected = !selected;
    break;
  }

  return false;
}

#endif
