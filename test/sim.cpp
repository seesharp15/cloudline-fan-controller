// Definitions behind stubs/*.h and sim.h.
#include "sim.h"

#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "stubs/U8g2lib.h"
#include "stubs/Wire.h"

// ---------------------------------------------------------------------------
// Arduino / AVR globals
// ---------------------------------------------------------------------------
uint8_t TCCR1A, TCCR1B, PCICR, PCIFR, PCMSK2;
uint16_t ICR1, OCR1A, TCNT1;
SerialStub Serial;
TwoWireStub Wire;

namespace sim {
Pin pins[20];
uint64_t nowUs = 0;
void (*extIsr[2])() = {nullptr, nullptr};
double tachHz = 0;
unsigned toneHz = 0;
static double nextTachUs = 0;

void reset() {
  for (Pin &p : pins) p = Pin();
  nowUs = 0;
  extIsr[0] = extIsr[1] = nullptr;
  TCCR1A = TCCR1B = PCICR = PCIFR = PCMSK2 = 0;
  ICR1 = OCR1A = TCNT1 = 0;
  tachHz = 0;
  nextTachUs = 0;
  toneHz = 0;
  Serial = SerialStub();
  Wire = TwoWireStub();
  Wire.present[0x3C] = true;  // the OLED
}

// Fire tach edges that fall inside (nowUs, untilUs]. The ISR is attached to
// D2 FALLING; the pin level itself is not used by the firmware.
static void runTachUntil(uint64_t untilUs) {
  if (tachHz <= 0) { nextTachUs = 0; return; }
  const double period = 1e6 / tachHz;
  if (nextTachUs <= (double)nowUs) nextTachUs = (double)nowUs + period;
  while (nextTachUs <= (double)untilUs) {
    nowUs = (uint64_t)nextTachUs;
    if (extIsr[0]) extIsr[0]();
    nextTachUs += period;
  }
}

void advance(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    const uint64_t end = nowUs + 1000;
    runTachUntil(end);
    nowUs = end;
    loop();
  }
}

static void setEncoder(int a, int b) {
  pins[3].external = (int8_t)a;
  pins[4].external = (int8_t)b;
  isr_PCINT2_vect();
}

// Decoder-positive order from rest (A=1,B=1): 01, 00, 10, 11.
static const int kSeqPos[4][2] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
static const int kSeqNeg[4][2] = {{1, 0}, {0, 0}, {0, 1}, {1, 1}};

void turn(int detents, uint32_t msBetween) {
  const int n = detents < 0 ? -detents : detents;
  const int (*seq)[2] = detents < 0 ? kSeqNeg : kSeqPos;
  for (int d = 0; d < n; d++) {
    for (int e = 0; e < 4; e++) {
      setEncoder(seq[e][0], seq[e][1]);
      nowUs += 250;  // edges ~0.25 ms apart within a detent
    }
    advance(msBetween ? msBetween : 1);
  }
}

void bounceDetent(int dir) {
  const int (*seq)[2] = dir < 0 ? kSeqNeg : kSeqPos;
  int prevA = 1, prevB = 1;
  for (int e = 0; e < 4; e++) {
    // Chatter: new level, back to old level, new level again.
    setEncoder(seq[e][0], seq[e][1]);
    setEncoder(prevA, prevB);
    setEncoder(seq[e][0], seq[e][1]);
    prevA = seq[e][0];
    prevB = seq[e][1];
    nowUs += 300;
  }
  advance(50);
}

void hold(uint8_t pin) { pins[pin].external = 0; }
void release(uint8_t pin) { pins[pin].external = -1; }

void press(uint8_t pin, uint32_t holdMs) {
  hold(pin);
  advance(holdMs);
  release(pin);
  advance(60);
}

bool pwmConnected() { return (TCCR1A & _BV(COM1A1)) != 0; }
uint16_t pwmTop() { return ICR1; }

uint16_t gateDutyPermille() {
  if (!pwmConnected()) return pins[9].out ? 1000 : 0;
  // Fast PWM non-inverting: HIGH for OCR1A + 1 of ICR1 + 1 ticks.
  return (uint16_t)(((uint32_t)(OCR1A + 1u) * 1000u + (ICR1 + 1u) / 2u) / (ICR1 + 1u));
}

void serial(const char *line) {
  Serial.inject(line);
  Serial.inject("\n");
  advance(10);  // the firmware reads at most 16 characters per loop pass
}

std::string takeSerial() {
  std::string o = Serial.out;
  Serial.out.clear();
  return o;
}
}  // namespace sim

// ---------------------------------------------------------------------------
// Fake U8g2 fonts and frame tools
// ---------------------------------------------------------------------------
// Metrics of the real U8g2 fonts (BDF bounding boxes of the X11 fixed fonts).
const FakeFont u8g2_font_6x10_tr = {"6x10", 6, 7, 8, 2, false};
const FakeFont u8g2_font_7x13B_tr = {"7x13B", 7, 9, 11, 2, true};
const FakeFont u8g2_font_5x8_tr = {"5x8", 5, 6, 7, 1, false};

bool FakeFrame::hasText(const char *needle) const {
  for (const FakeText &t : texts)
    if (t.str.find(needle) != std::string::npos) return true;
  return false;
}

std::string FakeFrame::allText() const {
  std::string s;
  for (const FakeText &t : texts) { s += t.str; s += " | "; }
  return s;
}

std::vector<std::string> FakeFrame::check() const {
  std::vector<std::string> problems;
  char msg[160];
  for (size_t i = 0; i < texts.size(); i++) {
    const FakeText &t = texts[i];
    const int w = (int)t.str.size() * t.font->width;
    if (t.x + w > 128 || t.baseline - t.font->ascent < 0 || t.baseline > 64) {
      snprintf(msg, sizeof msg, "text \"%s\" at (%d,%d) %s exceeds the 128x64 screen",
               t.str.c_str(), t.x, t.baseline, t.font->name);
      problems.push_back(msg);
    }
    for (size_t j = i + 1; j < texts.size(); j++) {
      const FakeText &u = texts[j];
      const int uw = (int)u.str.size() * u.font->width;
      const bool xo = t.x < u.x + uw && u.x < t.x + w;
      const bool yo = t.baseline - t.font->capH < u.baseline &&
                      u.baseline - u.font->capH < t.baseline;
      if (xo && yo) {
        snprintf(msg, sizeof msg, "text \"%s\" overlaps \"%s\"", t.str.c_str(), u.str.c_str());
        problems.push_back(msg);
      }
    }
  }
  return problems;
}

static std::string xmlEscape(const std::string &s) {
  std::string o;
  for (char c : s) {
    if (c == '&') o += "&amp;";
    else if (c == '<') o += "&lt;";
    else if (c == '>') o += "&gt;";
    else o += c;
  }
  return o;
}

// OLED-style screenshot: lit pixels as cells on a black panel, text drawn
// with a monospace face scaled to the U8g2 font's cell width.
bool FakeFrame::writeSvg(const char *path, int scale) const {
  FILE *f = fopen(path, "w");
  if (!f) return false;
  const int pad = 6 * scale;
  const int W = 128 * scale + 2 * pad, H = 64 * scale + 2 * pad;
  const char *on = "#e8f4ff";
  fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\" "
          "shape-rendering=\"crispEdges\">\n",
          W, H, W, H);
  fprintf(f, "<rect width=\"%d\" height=\"%d\" rx=\"%d\" fill=\"#1b1f24\"/>\n", W, H, 3 * scale);
  fprintf(f, "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#000\"/>\n", pad, pad,
          128 * scale, 64 * scale);
  // Merge horizontal runs of lit pixels into single rects.
  for (int y = 0; y < 64; y++) {
    int x = 0;
    while (x < 128) {
      if (!px[y][x]) { x++; continue; }
      int x2 = x;
      while (x2 < 128 && px[y][x2]) x2++;
      fprintf(f, "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"%s\"/>\n",
              pad + x * scale, pad + y * scale, (x2 - x) * scale, scale, on);
      x = x2;
    }
  }
  for (const FakeText &t : texts) {
    // Monospace advance is ~0.6 em, so font-size = cell width / 0.6.
    const double size = t.font->width / 0.6 * scale;
    fprintf(f,
            "<text x=\"%d\" y=\"%d\" font-family=\"Menlo, Consolas, 'DejaVu Sans Mono', monospace\" "
            "font-size=\"%.1f\" textLength=\"%d\" lengthAdjust=\"spacingAndGlyphs\" "
            "font-weight=\"%s\" fill=\"%s\" xml:space=\"preserve\">%s</text>\n",
            pad + t.x * scale, pad + t.baseline * scale, size,
            (int)t.str.size() * t.font->width * scale, t.font->bold ? "bold" : "normal",
            t.color == 0 ? "#000" : on, xmlEscape(t.str).c_str());
  }
  fprintf(f, "</svg>\n");
  fclose(f);
  return true;
}
