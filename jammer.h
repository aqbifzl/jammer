#ifndef JAMMER_H
#define JAMMER_H

#include <RF24.h>
#include <nRF24L01.h>
#include <stdint.h>
#include <vector>

class Jammer {
public:
  using LoopFunction = void (Jammer::*)();

  struct Mode {
    const char *name;
    LoopFunction loop_func;
  };

private:
  RF24 *_nrf1 = nullptr;
  RF24 *_nrf2 = nullptr;
  bool _active = false;
  int mode_cache;

  Jammer();

  void setup_radio(RF24 *radio, uint8_t ch);

  void loop_ble();
  void loop_blea();
  void loop_bt();
  void loop_all();
  void loop_zigbee();
  void loop_nrf();
  void loop_ble_rand();
  void loop_wifi();

public:
  static Jammer &instance() {
    static Jammer instance;
    return instance;
  }

  void init_radios(RF24 *r1, RF24 *r2);
  void start();
  void stop();

  void update();

  bool is_active() const { return _active; }

  const char *get_mode_name(int index) const;
  static const std::vector<Mode> &get_modes_vector();
  int get_mode_count() const;
};

#endif // JAMMER_H
