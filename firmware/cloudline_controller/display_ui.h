// display_ui.h - screen layouts for the 128x64 SH1106 OLED.
//
// The draw functions are templates over the display type so the exact same
// code runs on the Nano (with a U8G2 object) and in the host test harness
// (with a fake that renders screenshots, see tests/). Only these U8g2 calls
// are used: setFont, setFontMode, setDrawColor, drawStr, getStrWidth,
// drawBox, drawFrame, drawHLine, drawVLine.
#pragma once

#include <stdint.h>
#include <stdio.h>

namespace ui {

enum class Screen : uint8_t { Main, Calibrating, CalResult, Diagnostics };

struct State {
  Screen screen = Screen::Main;

  // Main screen
  bool fanOn = false;
  uint8_t settingPct = 0;     // user setting, 0..100
  uint8_t actualPct = 0;      // tach speed relative to calibrated max
  uint16_t rpm = 0;
  bool tachFault = false;     // fan commanded on but no tach pulses
  bool calibrated = false;    // a calibration is stored in EEPROM
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
};

const uint8_t kWidth = 128;
const uint8_t kHeight = 64;

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

template <class G>
void drawRightAligned(G &g, uint8_t rightX, uint8_t baseline, const char *s) {
  g.drawStr(rightX - g.getStrWidth(s), baseline, s);
}

// Title bar: filled (inverted) when `inverted`, outlined otherwise.
template <class G>
void drawTitle(G &g, const char *left, const char *right, bool inverted) {
  g.setFontMode(1);
  g.setDrawColor(1);
  if (inverted) g.drawBox(0, 0, kWidth, 14);
  else g.drawFrame(0, 0, kWidth, 14);
  g.setDrawColor(inverted ? 0 : 1);
  g.setFont(u8g2_font_7x13B_tf);
  g.drawStr(3, 11, left);
  if (right) {
    g.setFont(u8g2_font_6x10_tf);
    drawRightAligned(g, kWidth - 3, 10, right);
  }
  g.setDrawColor(1);
}

template <class G>
void drawMain(G &g, const State &s) {
  char buf[24];

  snprintf(buf, sizeof buf, "%u RPM", (unsigned)s.rpm);
  drawTitle(g, s.fanOn ? "FAN ON" : "FAN OFF", buf, s.fanOn);

  // Two labelled bars: SET (user setting) and ACT (tach feedback).
  g.setFont(u8g2_font_6x10_tf);
  const uint8_t barX = 22, barW = 80, barH = 12;
  const uint8_t setY = 18, actY = 35;

  g.drawStr(0, setY + 9, "SET");
  drawBar(g, barX, setY, barW, barH, s.settingPct, -1);
  snprintf(buf, sizeof buf, "%u%%", (unsigned)s.settingPct);
  drawRightAligned(g, kWidth, setY + 9, buf);

  g.drawStr(0, actY + 9, "ACT");
  drawBar(g, barX, actY, barW, barH, s.tachFault ? 0 : s.actualPct,
          s.fanOn ? (int8_t)s.settingPct : (int8_t)-1);
  if (s.tachFault) snprintf(buf, sizeof buf, "--%%");
  else snprintf(buf, sizeof buf, "%u%%", (unsigned)s.actualPct);
  drawRightAligned(g, kWidth, actY + 9, buf);

  // Footer: the most useful hint for the current situation.
  const char *hint;
  if (s.knobHeldMs >= 600 && s.fanOn) {
    const uint32_t left = s.knobHeldMs >= s.calHoldMs ? 0 : s.calHoldMs - s.knobHeldMs;
    snprintf(buf, sizeof buf, "Hold for CAL: %lus", (unsigned long)((left + 999) / 1000));
    hint = buf;
  } else if (s.tachFault) {
    hint = "! NO TACH / STALLED";
  } else if (!s.fanOn) {
    hint = "Press knob to start";
  } else if (!s.calibrated) {
    hint = "Hold knob 3s: CAL";
  } else {
    hint = "Turn knob: speed";
  }
  g.drawStr(0, 61, hint);
}

template <class G>
void drawCalibrating(G &g, const State &s) {
  char buf[24];
  drawTitle(g, "CALIBRATE", nullptr, true);
  g.setFont(u8g2_font_6x10_tf);
  g.drawStr(0, 26, "Fan at 100%");
  snprintf(buf, sizeof buf, "%s %us", s.calSampling ? "Measuring" : "Spin-up", (unsigned)s.calSecondsLeft);
  g.drawStr(0, 38, buf);
  drawBar(g, 0, 42, kWidth, 9, s.calProgressPct, -1);
  snprintf(buf, sizeof buf, "%u RPM", (unsigned)s.calRpm);
  g.drawStr(0, 62, buf);
  drawRightAligned(g, kWidth, 62, "BAK/knob=X");
}

template <class G>
void drawCalResult(G &g, const State &s) {
  char buf[24];
  drawTitle(g, s.calOk ? "CAL SAVED" : "CAL FAILED", nullptr, true);
  g.setFont(u8g2_font_6x10_tf);
  if (s.calOk) {
    g.drawStr(0, 30, "100% =");
    snprintf(buf, sizeof buf, "%u RPM", (unsigned)s.calRpm);
    g.setFont(u8g2_font_7x13B_tf);
    g.drawStr(44, 31, buf);
    g.setFont(u8g2_font_6x10_tf);
    g.drawStr(0, 48, "Stored in EEPROM.");
  } else {
    g.drawStr(0, 30, "No stable tach");
    g.drawStr(0, 42, "signal at 100%.");
    g.drawStr(0, 54, "See VERIFICATION.md");
  }
}

template <class G>
void drawDiagnostics(G &g, const State &s) {
  char buf[28];
  drawTitle(g, "DIAG", s.fanOn ? "fan on" : "fan off", true);
  g.setFont(u8g2_font_5x8_tf);
  snprintf(buf, sizeof buf, "Tach %lu.%03lu Hz",
           (unsigned long)(s.tachMilliHz / 1000), (unsigned long)(s.tachMilliHz % 1000));
  g.drawStr(0, 23, buf);
  snprintf(buf, sizeof buf, "RPM  %u (%u p/rev)", (unsigned)s.rpm, (unsigned)s.pulsesPerRev);
  g.drawStr(0, 32, buf);
  snprintf(buf, sizeof buf, "Max  %u RPM %s", (unsigned)s.maxRpm, s.calibrated ? "(cal)" : "(dflt)");
  g.drawStr(0, 41, buf);
  snprintf(buf, sizeof buf, "Line %u.%u%% high", (unsigned)(s.dutyPermille / 10), (unsigned)(s.dutyPermille % 10));
  g.drawStr(0, 50, buf);
  if (s.gateMode == 2) snprintf(buf, sizeof buf, "Gate PWM %u/%u", (unsigned)s.pwmCompare, (unsigned)s.pwmTop);
  else snprintf(buf, sizeof buf, "Gate %s", s.gateMode == 1 ? "HIGH (line low)" : "LOW (line high)");
  g.drawStr(0, 59, buf);
  snprintf(buf, sizeof buf, "n=%lu", (unsigned long)s.tachPulses);
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
