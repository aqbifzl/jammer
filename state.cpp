#include "state.h"
#include <cstring>

bool ScannerSettings::get_spectrum_data(size_t index) {
  if (index >= CHANNELS) {
    return false;
  }

  return spectrum_data[index];
}

void ScannerSettings::set_spectrum_data(size_t index, bool value) {
  if (index >= CHANNELS) {
    return;
  }
  spectrum_data[index] = value;
}

void ScannerSettings::set_spectrum_data_all(const bool *data, size_t len) {
  if (!data) {
    return;
  }

  size_t count = (len < CHANNELS) ? len : CHANNELS;

  for (size_t i = 0; i < count; i++) {
    spectrum_data[i] = data[i];
  }
}

bool ScannerSettings::spectrum_changed(bool *b) {
  return memcmp(b, spectrum_data, CHANNELS) != 0;
}
