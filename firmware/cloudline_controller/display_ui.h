// display_ui.h - screen layouts for the 128x64 OLED.
//
// The draw functions are templates over the display type so the exact same
// code runs on the Nano (with a U8G2 object) and in the host test harness
// (with a fake that renders screenshots, see test/). Only these U8g2 calls
// are used: setFont, setFontMode, setDrawColor, drawStr, getStrWidth,
// drawBox, drawFrame, drawVLine.
//
// Every fixed string is kept in flash (PSTR / snprintf_P): on the ATmega328P
// string literals would otherwise be copied into the 2 KB of RAM at start-up.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
// Host build: flash and RAM are the same address space.
#ifndef PSTR
#define PSTR(s) (s)
#endif
#ifndef snprintf_P
#define snprintf_P snprintf
#endif
#ifndef strncpy_P
#define strncpy_P strncpy
#endif
#endif

namespace ui {

enum class Screen : uint8_t { Main, Calibrating, CalResult, Diagnostics };

struct State {
  Screen screen = Screen::Main;

  // Main screen
  bool fanOn = false;
  uint8_t settingPct = 0;     // user setting, 0..100
  uint8_t actualPct = 0;      // tach speed relative to the max RPM
  uint16_t rpm = 0;
  bool tachFault = false;     // fan commanded on but no tach pulses
  bool calibrated = false;    // max RPM known (config or calibration this session)
  uint32_t knobHeldMs = 0;    // > 0 while the knob is held (CAL countdown)
  uint32_t calHoldMs = 3000;

  // Calibration
  bool calSampling = false;   // false = spinning up, true = averaging
  uint8_t calSecondsLeft = 0;
  uint8_t calProgressPct = 0;
  bool calOk = false;
  uint16_t calRpm = 0;        // live RPM while running, result afterwards

  // Diagnostics
  uint32_t tachMilliHz = 0;
  uint32_t tachPulses = 0;
  uint8_t pulsesPerRev = 2;
  uint16_t dutyPermille = 0;  // duty on the fan speed line
  uint16_t maxRpm = 0;
  uint16_t pwmCompare = 0;
  uint16_t pwmTop = 0;
  uint8_t gateMode = 0;       // 0 = constant low, 1 = constant high, 2 = PWM
  bool inverted = true;       // FAN_PWM_INVERTED: gate HIGH pulls the line low
};

const uint8_t kWidth = 128;
const uint8_t kHeight = 64;

// Draw a string that lives in flash (PSTR).
template <class G>
void drawStrP(G &g, uint8_t x, uint8_t y, const char *flashStr) {
  char buf[22];
  strncpy_P(buf, flashStr, sizeof buf - 1);
  buf[sizeof buf - 1] = 0;
  g.drawStr(x, y, buf);
}

// Horizontal bar with tick marks at 25/50/75 %. `marker` (0..100, or < 0 for
// none) draws a vertical line across the bar, used for the target setting.
template <class G>
void drawBar(G &g, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t pct,
             int8_t marker) {
  g.setDrawColor(1);
  g.drawFrame(x, y, w, h);
  const uint8_t innerW = (uint8_t)(w - 4);
  const uint8_t fill = (uint8_t)(((uint16_t)innerW * (pct > 100 ? 100 : pct) + 50) / 100);
  if (fill > 0) g.drawBox(x + 2, y + 2, fill, h - 4);
  // Ticks are drawn in XOR so they show on both filled and empty segments.
  g.setDrawColor(2);
  for (uint8_t q = 1; q <= 3; q++) {
    const uint8_t tx = (uint8_t)(x + 2 + (uint16_t)innerW * q / 4);
    g.drawVLine(tx, y + 2, 2);
    g.drawVLine(tx, y + h - 4, 2);
  }
  if (marker >= 0) {
    const uint8_t mx = (uint8_t)(x + 2 + ((uint16_t)innerW * (uint8_t)marker + 50) / 100);
    g.drawVLine(mx > x + w - 3 ? x + w - 3 : mx, y + 1, h - 2);
  }
  g.setDrawColor(1);
}

// `s` is a RAM string.
template <class G>
void drawRightAligned(G &g, uint8_t rightX, uint8_t baseline, const char *s) {
  g.drawStr(rightX - g.getStrWidth(s), baseline, s);
}

// Title bar: filled (inverted) when `inverted`, outlined otherwise.
// `leftP` is a flash string, `right` a RAM string or nullptr.
template <class G>
void drawTitle(G &g, const char *leftP, const char *right, bool inverted) {
  g.setFontMode(1);
  g.setDrawColor(1);
  if (inverted) g.drawBox(0, 0, kWidth, 14);
  else g.drawFrame(0, 0, kWidth, 14);
  g.setDrawColor(inverted ? 0 : 1);
  g.setFont(u8g2_font_7x13B_tr);
  drawStrP(g, 3, 11, leftP);
  if (right) {
    g.setFont(u8g2_font_6x10_tr);
    drawRightAligned(g, kWidth - 3, 10, right);
  }
  g.setDrawColor(1);
}

template <class G>
void drawMain(G &g, const State &s) {
  char buf[22];

  snprintf_P(buf, sizeof buf, PSTR("%u RPM"), (unsigned)s.rpm);
  drawTitle(g, s.fanOn ? PSTR("FAN ON") : PSTR("FAN OFF"), buf, s.fanOn);

  // Two labelled bars: SET (user setting) and ACT (tach feedback).
  g.setFont(u8g2_font_6x10_tr);
  const uint8_t barX = 22, barW = 80, barH = 12;
  const uint8_t setY = 18, actY = 35;

  drawStrP(g, 0, setY + 9, PSTR("SET"));
  drawBar(g, barX, setY, barW, barH, s.settingPct, -1);
  snprintf_P(buf, sizeof buf, PSTR("%u%%"), (unsigned)s.settingPct);
  drawRightAligned(g, kWidth, setY + 9, buf);

  // ACT is unknown ("--") when the tach is silent or the max RPM is unknown.
  const bool actKnown = !s.tachFault && s.calibrated;
  drawStrP(g, 0, actY + 9, PSTR("ACT"));
  drawBar(g, barX, actY, barW, barH, actKnown ? s.actualPct : 0,
          s.fanOn ? (int8_t)s.settingPct : (int8_t)-1);
  if (actKnown) snprintf_P(buf, sizeof buf, PSTR("%u%%"), (unsigned)s.actualPct);
  else snprintf_P(buf, sizeof buf, PSTR("--%%"));
  drawRightAligned(g, kWidth, actY + 9, buf);

  // Footer: the most useful hint for the current situation.
  if (s.knobHeldMs >= 600 && s.fanOn) {
    const uint32_t left = s.knobHeldMs >= s.calHoldMs ? 0 : s.calHoldMs - s.knobHeldMs;
    const uint32_t secs = (left + 999) / 1000;
    snprintf_P(buf, sizeof buf, PSTR("Hold for CAL: %us"), (unsigned)(secs > 99 ? 99 : secs));
    g.drawStr(0, 61, buf);
  } else if (s.tachFault) {
    drawStrP(g, 0, 61, PSTR("! NO TACH / STALLED"));
  } else if (!s.fanOn) {
    drawStrP(g, 0, 61, PSTR("Press knob to start"));
  } else if (!s.calibrated) {
    drawStrP(g, 0, 61, PSTR("Hold knob 3s: CAL"));
  } else {
    drawStrP(g, 0, 61, PSTR("Turn knob: speed"));
  }
}

template <class G>
void drawCalibrating(G &g, const State &s) {
  char buf[22];
  drawTitle(g, PSTR("CALIBRATE"), nullptr, true);
  g.setFont(u8g2_font_6x10_tr);
  drawStrP(g, 0, 26, PSTR("Fan at 100%"));
  if (s.calSampling) snprintf_P(buf, sizeof buf, PSTR("Measuring %us"), (unsigned)s.calSecondsLeft);
  else snprintf_P(buf, sizeof buf, PSTR("Spin-up %us"), (unsigned)s.calSecondsLeft);
  g.drawStr(0, 38, buf);
  drawBar(g, 0, 42, kWidth, 9, s.calProgressPct, -1);
  snprintf_P(buf, sizeof buf, PSTR("%u RPM"), (unsigned)s.calRpm);
  g.drawStr(0, 62, buf);
  snprintf_P(buf, sizeof buf, PSTR("BAK/knob=X"));
  drawRightAligned(g, kWidth, 62, buf);
}

template <class G>
void drawCalResult(G &g, const State &s) {
  char buf[22];
  drawTitle(g, s.calOk ? PSTR("CAL DONE") : PSTR("CAL FAILED"), nullptr, true);
  g.setFont(u8g2_font_6x10_tr);
  if (s.calOk) {
    drawStrP(g, 0, 30, PSTR("100% ="));
    snprintf_P(buf, sizeof buf, PSTR("%u RPM"), (unsigned)s.calRpm);
    g.setFont(u8g2_font_7x13B_tr);
    g.drawStr(44, 31, buf);
    g.setFont(u8g2_font_6x10_tr);
    drawStrP(g, 0, 46, PSTR("Kept until power-off."));
    drawStrP(g, 0, 58, PSTR("Serial: how to keep"));
  } else {
    drawStrP(g, 0, 30, PSTR("No stable tach"));
    drawStrP(g, 0, 42, PSTR("signal at 100%."));
    drawStrP(g, 0, 54, PSTR("See README trouble."));
  }
}

template <class G>
void drawDiagnostics(G &g, const State &s) {
  char buf[26];
  snprintf_P(buf, sizeof buf, s.fanOn ? PSTR("fan on") : PSTR("fan off"));
  drawTitle(g, PSTR("DIAG"), buf, true);
  g.setFont(u8g2_font_5x8_tr);
  snprintf_P(buf, sizeof buf, PSTR("Tach %lu.%03lu Hz"),
             (unsigned long)(s.tachMilliHz / 1000), (unsigned long)(s.tachMilliHz % 1000));
  g.drawStr(0, 23, buf);
  snprintf_P(buf, sizeof buf, PSTR("RPM  %u (%u p/rev)"), (unsigned)s.rpm, (unsigned)s.pulsesPerRev);
  g.drawStr(0, 32, buf);
  if (s.calibrated) snprintf_P(buf, sizeof buf, PSTR("Max  %u RPM"), (unsigned)s.maxRpm);
  else snprintf_P(buf, sizeof buf, PSTR("Max  unknown (CAL)"));
  g.drawStr(0, 41, buf);
  snprintf_P(buf, sizeof buf, PSTR("Line %u.%u%% high"), (unsigned)(s.dutyPermille / 10),
             (unsigned)(s.dutyPermille % 10));
  g.drawStr(0, 50, buf);
  if (s.gateMode == 2)
    snprintf_P(buf, sizeof buf, PSTR("Gate PWM %u/%u"), (unsigned)s.pwmCompare, (unsigned)s.pwmTop);
  else if (s.gateMode == 1)
    snprintf_P(buf, sizeof buf, s.inverted ? PSTR("Gate HIGH (stopped)") : PSTR("Gate HIGH (full)"));
  else
    snprintf_P(buf, sizeof buf, s.inverted ? PSTR("Gate LOW (full)") : PSTR("Gate LOW (stopped)"));
  g.drawStr(0, 59, buf);
  snprintf_P(buf, sizeof buf, PSTR("n=%lu"), (unsigned long)s.tachPulses);
  drawRightAligned(g, kWidth, 23, buf);
}

template <class G>
void draw(G &g, const State &s) {
  switch (s.screen) {
    case Screen::Main: drawMain(g, s); break;
    case Screen::Calibrating: drawCalibrating(g, s); break;
    case Screen::CalResult: drawCalResult(g, s); break;
    case Screen::Diagnostics: drawDiagnostics(g, s); break;
  }
}

}  // namespace ui
