#ifndef PAGE_H
#define PAGE_H

#include "kbd.h"

class Page {
public:
  virtual ~Page() {}
  virtual void on_enter() {}
  virtual void on_exit() {}

  virtual void handle_input(InputEvent *ev) = 0;
  virtual void draw() = 0;
};

#endif
