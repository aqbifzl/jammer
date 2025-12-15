#ifndef TX_PAGE_H
#define TX_PAGE_H

#include "menu_option.h"
#include "page.h"
#include "state.h"
#include <vector>

class TXPage : public Page {
private:
  int highlighted;
  bool selected;
  std::vector<MenuOption> options;

  TXPage();
  int get_highlighted_value();
  void set_highlighted_value(int val);

public:
  static TXPage *instance() {
    static TXPage sp;
    return &sp;
  }
  void draw() override;
  void handle_input(InputEvent *ev) override;
  void update() override;
};

#endif
