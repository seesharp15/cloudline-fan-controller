// Host-side Wire.h: records the settings the firmware applies and answers
// address probes for the devices listed in `present`.
#pragma once

#include <stdint.h>

class TwoWireStub {
 public:
  void begin() { begun = true; }
  void setClock(uint32_t hz) { clock = hz; }
  void setWireTimeout(uint32_t us, bool resetOnTimeout) {
    timeoutUs = us;
    resetWithTimeout = resetOnTimeout;
  }
  void beginTransmission(uint8_t addr) { addr_ = addr; }
  uint8_t endTransmission() { return (addr_ < 128 && present[addr_]) ? 0 : 2; }  // 2 = NACK on address
  bool getWireTimeoutFlag() const { return timeoutFlag; }
  void clearWireTimeoutFlag() { timeoutFlag = false; }

  bool begun = false;
  uint32_t clock = 0;
  uint32_t timeoutUs = 0;
  bool resetWithTimeout = false;
  bool present[128] = {};
  bool timeoutFlag = false;  // set by a test to simulate a stuck bus

 private:
  uint8_t addr_ = 0;
};
extern TwoWireStub Wire;
