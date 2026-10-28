// Host-side EEPROM: 1 KB of bytes (0xFF when erased), loadable from a file so
// a test can "power-cycle" the controller and check what survived.
#pragma once

#include <stdint.h>
#include <string.h>

struct EEPROMClass {
  uint8_t bytes[1024];
  unsigned writes = 0;  // counts bytes that actually changed

  EEPROMClass() { memset(bytes, 0xFF, sizeof bytes); }

  template <class T>
  T &get(int addr, T &value) {
    memcpy(&value, bytes + addr, sizeof(T));
    return value;
  }
  template <class T>
  const T &put(int addr, const T &value) {
    const uint8_t *p = (const uint8_t *)&value;
    for (unsigned i = 0; i < sizeof(T); i++) {
      if (bytes[addr + i] != p[i]) {
        bytes[addr + i] = p[i];
        writes++;
      }
    }
    return value;
  }
};

extern EEPROMClass EEPROM;
