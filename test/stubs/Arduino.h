// Host-side stand-in for the parts of Arduino.h / avr/io.h the sketch uses.
// Time, pins and the Timer1 / pin-change registers are plain variables that
// the simulation tests read and drive (see sim.h).
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string>

#define F_CPU 16000000UL

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define FALLING 2
#define RISING 3
#define CHANGE 1
#define LED_BUILTIN 13

static const uint8_t SDA = 18;  // A4
static const uint8_t SCL = 19;  // A5

#define _BV(b) (1u << (b))

// Timer1 bits (ATmega328P)
#define COM1A1 7
#define COM1A0 6
#define WGM11 1
#define WGM10 0
#define WGM13 4
#define WGM12 3
#define CS10 0

namespace sim {
struct Pin {
  uint8_t mode = INPUT;
  uint8_t out = LOW;        // PORT bit (output level, or pull-up when INPUT)
  int8_t external = -1;     // level forced by the outside world, -1 = floating
  int8_t outWhenMadeOutput = -1;  // PORT level at the moment pinMode(OUTPUT) ran
};
extern Pin pins[20];
extern uint64_t nowUs;
extern void (*extIsr[2])();
}  // namespace sim

extern uint8_t TCCR1A, TCCR1B, PCICR, PCIFR, PCMSK2;
extern uint16_t ICR1, OCR1A, TCNT1;

// 32-bit like on AVR, so the firmware sees the same wrap-arounds
// (micros() every ~71.6 min, millis() every ~49.7 days).
inline uint32_t millis() { return (uint32_t)(sim::nowUs / 1000u); }
inline uint32_t micros() { return (uint32_t)sim::nowUs; }
inline void delay(unsigned long ms) { sim::nowUs += (uint64_t)ms * 1000u; }

inline void pinMode(uint8_t p, uint8_t m) {
  if (m == OUTPUT && sim::pins[p].mode != OUTPUT) sim::pins[p].outWhenMadeOutput = (int8_t)sim::pins[p].out;
  sim::pins[p].mode = m;
  if (m == INPUT_PULLUP) sim::pins[p].out = HIGH;
  if (m == INPUT) sim::pins[p].out = LOW;
}
inline void digitalWrite(uint8_t p, uint8_t v) { sim::pins[p].out = v ? HIGH : LOW; }
inline int digitalRead(uint8_t p) {
  const sim::Pin &pin = sim::pins[p];
  if (pin.mode == OUTPUT) return pin.out;
  if (pin.external >= 0) return pin.external;
  return (pin.mode == INPUT_PULLUP || pin.out) ? HIGH : LOW;  // pull-up or floating-high
}

inline void noInterrupts() {}
inline void interrupts() {}

inline uint8_t digitalPinToInterrupt(uint8_t p) { return p == 2 ? 0 : (p == 3 ? 1 : 255); }
inline void attachInterrupt(uint8_t n, void (*f)(), int) { if (n < 2) sim::extIsr[n] = f; }

// Pin-change helpers for D0..D7 (port D, PCINT16..23, PCICR bit 2).
#define digitalPinToPCMSK(p) (&PCMSK2)
#define digitalPinToPCMSKbit(p) (p)
#define digitalPinToPCICRbit(p) 2

// ISR(PCINT2_vect) becomes an ordinary function the tests can call.
#define ISR(vec) void isr_##vec()

// tone() is only used by the Wokwi build (fake tach); record the last call.
namespace sim { extern unsigned toneHz; }
inline void tone(uint8_t, unsigned int hz) { sim::toneHz = hz; }
inline void noTone(uint8_t) { sim::toneHz = 0; }

// Flash strings: on the host they are ordinary strings.
class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))
#define DEC 10
#define HEX 16

// Serial: input is injected by the tests, output is captured.
class SerialStub {
 public:
  void begin(unsigned long b) { baud = b; }
  int available() { return (int)(in.size() - pos); }
  int read() { return pos < in.size() ? (unsigned char)in[pos++] : -1; }
  void inject(const char *s) { in += s; }

  size_t print(const __FlashStringHelper *s) { return print(reinterpret_cast<const char *>(s)); }
  size_t print(const char *s) { out += s; return strlen(s); }
  size_t print(char c) { out += c; return 1; }
  size_t print(unsigned char v, int base = DEC) { return num(v, base); }
  size_t print(int v, int base = DEC) { return num(v, base); }
  size_t print(unsigned int v, int base = DEC) { return num(v, base); }
  size_t print(long v, int base = DEC) { return num(v, base); }
  size_t print(unsigned long v, int base = DEC) { return num((long long)v, base); }
  template <class T>
  size_t println(T v) { size_t n = print(v); out += "\r\n"; return n + 2; }
  template <class T>
  size_t println(T v, int base) { size_t n = print(v, base); out += "\r\n"; return n + 2; }
  size_t println() { out += "\r\n"; return 2; }

  unsigned long baud = 0;
  std::string in, out;
  size_t pos = 0;

 private:
  size_t num(long long v, int base) {
    char b[24];
    snprintf(b, sizeof b, base == HEX ? "%llX" : "%lld", v);
    out += b;
    return strlen(b);
  }
};
extern SerialStub Serial;
