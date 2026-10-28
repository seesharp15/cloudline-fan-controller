// Host-side fake of the U8g2 API subset the firmware uses.
//
// Boxes, frames and lines are rasterised into a 128x64 bitmap with the same
// draw-colour rules as U8g2 (0 = clear, 1 = set, 2 = XOR). Text is recorded
// with fixed-width font metrics matching the real U8g2 fonts, which lets the
// tests check layout (bounds and overlaps) and export SVG "screenshots".
// Coordinates are uint8_t, as with U8g2 on AVR, so layout arithmetic that
// would wrap on the Nano also wraps here.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

typedef uint8_t u8g2_uint_t;
#define U8X8_PIN_NONE 255
#define U8G2_R0 nullptr

struct FakeFont {
  const char *name;
  uint8_t width;    // advance per glyph (all fonts used are monospaced)
  uint8_t capH;     // capital-letter height
  uint8_t ascent;   // tallest glyph above the baseline
  uint8_t descent;  // lowest glyph below the baseline
  bool bold;
};

extern const FakeFont u8g2_font_6x10_tr;
extern const FakeFont u8g2_font_7x13B_tr;
extern const FakeFont u8g2_font_5x8_tr;

struct FakeText {
  int x, baseline, color;
  std::string str;
  const FakeFont *font;
};

struct FakeFrame {
  uint8_t px[64][128];
  std::vector<FakeText> texts;
  bool hasText(const char *needle) const;
  std::string allText() const;
  // Layout problems found in this frame (empty = OK).
  std::vector<std::string> check() const;
  bool writeSvg(const char *path, int scale) const;
};

// One fake serves every buffer mode. Page mode (firstPage/nextPage) runs
// the drawing code kPages times like the real 256-byte page buffer does,
// and commits the frame after the last pass.
class FakeU8g2 {
 public:
  FakeU8g2(const void *, uint8_t) {}
  static const int kPages = 4;

  void firstPage() { clearBuffer(); pass = 0; }
  bool nextPage() {
    if (++pass < kPages) { clearBuffer(); return true; }
    sendBuffer();
    return false;
  }

  void setI2CAddress(uint8_t a) { i2cAddress = a; }
  void setBusClock(uint32_t hz) { busClock = hz; }
  bool begin() { begun = true; clearBuffer(); return true; }

  void clearBuffer() { memset(cur.px, 0, sizeof cur.px); cur.texts.clear(); }
  void sendBuffer() { last = cur; frames++; }

  void setFont(const FakeFont &f) { font = &f; }
  void setFontMode(uint8_t) {}
  void setDrawColor(uint8_t c) { color = c; }

  u8g2_uint_t getStrWidth(const char *s) const {
    return (u8g2_uint_t)(strlen(s) * font->width);
  }
  u8g2_uint_t drawStr(u8g2_uint_t x, u8g2_uint_t y, const char *s) {
    cur.texts.push_back({x, y, color, s, font});
    return getStrWidth(s);
  }

  void drawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) {
    for (int j = 0; j < h; j++)
      for (int i = 0; i < w; i++) plot(x + i, y + j);
  }
  void drawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) {
    if (w == 0 || h == 0) return;
    drawHLine(x, y, w);
    if (h > 1) drawHLine(x, y + h - 1, w);
    if (h > 2) {
      drawVLine(x, y + 1, h - 2);
      if (w > 1) drawVLine(x + w - 1, y + 1, h - 2);
    }
  }
  void drawHLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w) {
    for (int i = 0; i < w; i++) plot(x + i, y);
  }
  void drawVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t h) {
    for (int j = 0; j < h; j++) plot(x, y + j);
  }

  // Test access
  FakeFrame cur, last;
  unsigned frames = 0;
  unsigned outOfBounds = 0;
  uint8_t i2cAddress = 0;
  uint32_t busClock = 0;
  bool begun = false;
  int pass = 0;

 private:
  void plot(int x, int y) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) { outOfBounds++; return; }
    uint8_t &p = cur.px[y][x];
    p = color == 2 ? (uint8_t)!p : color;
  }
  const FakeFont *font = &u8g2_font_6x10_tr;
  uint8_t color = 1;
};

typedef FakeU8g2 U8G2_SH1106_128X64_NONAME_F_HW_I2C;
typedef FakeU8g2 U8G2_SH1106_128X64_NONAME_2_HW_I2C;
typedef FakeU8g2 U8G2_SSD1306_128X64_NONAME_F_HW_I2C;
typedef FakeU8g2 U8G2_SSD1306_128X64_NONAME_2_HW_I2C;
