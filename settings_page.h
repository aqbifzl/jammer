#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include "menu_option.h"
#include "page.h"
#include "state.h"

class SettingsPage : public Page {
private:
  std::vector<MenuOption> options;
  int highlighted;
  bool selected;
  SettingsPage();

public:
  static SettingsPage *instance() {
    static SettingsPage sp;
    return &sp;
  }
  void draw() override;
  void handle_input(InputEvent *ev) override;
};

#endif
