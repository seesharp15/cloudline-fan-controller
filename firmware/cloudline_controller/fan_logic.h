// fan_logic.h - hardware-independent logic for the Cloudline controller.
//
// Nothing in this file touches Arduino APIs or AVR registers, so the same
// code is compiled into the firmware and into the host unit tests in test/.
// The hardware glue (pins, Timer1, interrupts, watchdog, display, serial)
// lives in app.cpp and only feeds inputs into, and reads outputs from, the
// classes and functions below.
#pragma once

#include <stdint.h>

namespace fan {

// ---------------------------------------------------------------------------
// EC11 quadrature decoder
// ---------------------------------------------------------------------------
// State = A<<1 | B, read with pull-ups (1 = contact open). The decoder sums
// the signed Gray-code transitions and only reports a step when the shaft
// comes back to a rest (detent) position, so contact bounce (+1 -1 +1 ...)
// cancels itself out and is never reported, and an aborted half turn is
// discarded.
//
//  edgesPerDetent = 4: one full quadrature cycle per click (EC11 with
//                      20 pulses / 20 detents). Rest state is A=1,B=1.
//  edgesPerDetent = 2: half a cycle per click (EC11 with 15 pulses /
//                      30 detents). Rest states are 11 and 00.
//
// Index = (previous << 2) | current. "Positive" order is
// 11 -> 01 -> 00 -> 10 -> 11 (A falls first). Invalid double jumps count 0.
static const int8_t kQuadTransition[16] = {
    0, -1, +1, 0,   // from 00
    +1, 0, 0, -1,   // from 01
    -1, 0, 0, +1,   // from 10
    0, +1, -1, 0,   // from 11
};

class QuadratureDecoder {
 public:
  explicit QuadratureDecoder(uint8_t edgesPerDetent = 4)
      : edges_(edgesPerDetent == 2 ? 2 : 4) {}

  // Feed the current pin levels; returns +1, -1 or 0 detents.
  int8_t update(bool a, bool b) {
    const uint8_t state = (uint8_t)((a ? 2 : 0) | (b ? 1 : 0));
    if (state == prev_) return 0;
    acc_ += kQuadTransition[(prev_ << 2) | state];
    prev_ = state;
    const bool rest = state == 3 || (edges_ == 2 && state == 0);
    if (!rest) return 0;
    // A clean detent sums to +/-edges. Anything less (bounce that did not
    // make it all the way, an aborted turn, or an invalid double jump caused
    // by a missed edge) is discarded rather than guessed at.
    int8_t step = 0;
    if (acc_ >= (int8_t)edges_) step = 1;
    else if (acc_ <= -(int8_t)edges_) step = -1;
    acc_ = 0;
    return step;
  }

  void reset(bool a, bool b) {
    prev_ = (uint8_t)((a ? 2 : 0) | (b ? 1 : 0));
    acc_ = 0;
  }

 private:
  uint8_t edges_;
  uint8_t prev_ = 3;
  int8_t acc_ = 0;
};

// ---------------------------------------------------------------------------
// Debounced push button with short-press and long-press events
// ---------------------------------------------------------------------------
// The raw level must be stable for debounceMs before it is accepted, so
// contact bounce can never produce more than one press. A Short event fires
// once, on release; a Long event fires once while still held and then
// suppresses the Short for that press.
enum class ButtonEvent : uint8_t { None, Short, Long };

class Button {
 public:
  Button(uint16_t debounceMs, uint32_t longMs)
      : debounceMs_(debounceMs), longMs_(longMs) {}

  // `pressed` is the raw level (true = contact closed). Call often.
  ButtonEvent update(bool pressed, uint32_t nowMs) {
    if (pressed != raw_) {
      raw_ = pressed;
      rawChangedMs_ = nowMs;
    }
    if (raw_ != stable_ && (uint32_t)(nowMs - rawChangedMs_) >= debounceMs_) {
      stable_ = raw_;
      if (stable_) {
        pressedMs_ = nowMs;
        longFired_ = false;
      } else if (!longFired_ && !suppress_) {
        return ButtonEvent::Short;
      } else {
        suppress_ = false;
      }
    }
    if (stable_ && !longFired_ && !suppress_ &&
        (uint32_t)(nowMs - pressedMs_) >= longMs_) {
      longFired_ = true;
      return ButtonEvent::Long;
    }
    return ButtonEvent::None;
  }

  // Ignore the current press entirely (e.g. a press that started at boot).
  void suppressUntilRelease() { suppress_ = stable_ || raw_; }

  bool held() const { return stable_; }
  uint32_t heldMs(uint32_t nowMs) const {
    return stable_ ? (uint32_t)(nowMs - pressedMs_) : 0;
  }

 private:
  uint16_t debounceMs_;
  uint32_t longMs_;
  bool raw_ = false;
  bool stable_ = false;
  bool longFired_ = false;
  bool suppress_ = false;
  uint32_t rawChangedMs_ = 0;
  uint32_t pressedMs_ = 0;
};

// ---------------------------------------------------------------------------
// Speed setting and PWM duty
// ---------------------------------------------------------------------------
inline int16_t clampI16(int16_t v, int16_t lo, int16_t hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

// Apply `detents` encoder steps to `setting`, using the larger step when the
// knob is being spun quickly (time since the previous detent < fastMs).
inline uint8_t applyDetents(uint8_t setting, int8_t detents,
                            uint32_t msSinceLastDetent, uint32_t fastMs,
                            uint8_t slowStep, uint8_t fastStep,
                            uint8_t minPct, uint8_t maxPct) {
  if (detents == 0) return setting;
  const int16_t step = (msSinceLastDetent < fastMs) ? fastStep : slowStep;
  int16_t v = (int16_t)setting + (int16_t)detents * step;
  return (uint8_t)clampI16(v, minPct, maxPct);
}

// Duty cycle on the fan's speed line in permille (0..1000), where 1000 means
// the line is released (high) all the time = full speed.
// Settings 1..100 map linearly onto (dutyMinPct, dutyMaxPct].
// OFF, or a setting of 0, is always 0 (line held low).
inline uint16_t settingToDutyPermille(uint8_t settingPct, bool on,
                                      uint8_t dutyMinPct, uint8_t dutyMaxPct) {
  if (!on || settingPct == 0) return 0;
  if (settingPct > 100) settingPct = 100;
  if (dutyMaxPct > 100) dutyMaxPct = 100;
  if (dutyMinPct > dutyMaxPct) dutyMinPct = dutyMaxPct;
  const uint16_t span = (uint16_t)(dutyMaxPct - dutyMinPct);
  return (uint16_t)(dutyMinPct * 10u + (uint32_t)span * settingPct / 10u);
}

// How Timer1 should drive the gate of Q1 for a given fan-line duty.
// Q1 pulls the fan line LOW while its gate is HIGH, so with `inverted` the
// gate duty is the complement of the fan-line duty.
struct GateDrive {
  enum Mode : uint8_t { ConstantLow, ConstantHigh, Pwm } mode;
  uint16_t compare;  // OCR1A value when mode == Pwm (gate HIGH for compare+1 ticks)
};

inline GateDrive gateDriveFor(uint16_t fanDutyPermille, uint16_t top,
                              bool inverted) {
  if (fanDutyPermille > 1000) fanDutyPermille = 1000;
  const uint16_t gatePermille =
      inverted ? (uint16_t)(1000 - fanDutyPermille) : fanDutyPermille;
  if (gatePermille == 0) return {GateDrive::ConstantLow, 0};
  if (gatePermille == 1000) return {GateDrive::ConstantHigh, top};
  // Fast PWM, non-inverting: pin HIGH for (OCR1A + 1) of (TOP + 1) ticks.
  uint32_t highTicks = ((uint32_t)gatePermille * (top + 1u) + 500u) / 1000u;
  if (highTicks < 1) highTicks = 1;
  if (highTicks > top) highTicks = top;  // keep at least one LOW tick
  return {GateDrive::Pwm, (uint16_t)(highTicks - 1u)};
}

// Timer1 TOP value for fast PWM mode 14 with the given prescaler.
inline uint16_t timer1TopFor(uint32_t cpuHz, uint16_t prescaler,
                             uint32_t pwmHz) {
  if (prescaler == 0) prescaler = 1;
  if (pwmHz == 0) pwmHz = 1;
  uint32_t ticks = cpuHz / ((uint32_t)prescaler * pwmHz);
  if (ticks < 2) ticks = 2;
  if (ticks > 65536UL) ticks = 65536UL;
  return (uint16_t)(ticks - 1u);
}

// ---------------------------------------------------------------------------
// Tachometer
// ---------------------------------------------------------------------------
// The tach ISR only increments a pulse counter and stores the micros() time
// of the most recent pulse. Every window the main loop hands both values to
// update(). The speed is computed from the time between the last pulse of the
// previous window and the last pulse of this one, so the result is exact
// (period based) rather than quantised to "pulses per window". With no new
// pulse for longer than the timeout the speed is reported as 0.
class TachEstimator {
 public:
  TachEstimator(uint8_t pulsesPerRev, uint32_t timeoutUs)
      : ppr_(pulsesPerRev ? pulsesPerRev : 1), timeoutUs_(timeoutUs) {}

  // Returns the latest raw RPM (0 when stopped).
  uint16_t update(uint32_t pulseCount, uint32_t lastPulseUs, uint32_t nowUs) {
    const uint32_t newPulses = pulseCount - refCount_;
    if (newPulses == 0) {
      if (!haveRef_ || (uint32_t)(nowUs - refUs_) > timeoutUs_) {
        rpm_ = 0;
        freqMilliHz_ = 0;
        haveRef_ = false;
      }
      return rpm_;
    }
    if (!haveRef_) {
      // First pulse(s) after a stop: just establish a reference edge.
      haveRef_ = true;
    } else {
      const uint32_t dt = lastPulseUs - refUs_;
      if (dt > 0) {
        // freq = newPulses / dt  -> in milli-Hz for the diagnostics page.
        const float hz = (float)newPulses * 1000000.0f / (float)dt;
        freqMilliHz_ = (uint32_t)(hz * 1000.0f + 0.5f);
        const float rpm = hz * 60.0f / (float)ppr_;
        rpm_ = rpm > 65535.0f ? 65535u : (uint16_t)(rpm + 0.5f);
      }
    }
    refCount_ = pulseCount;
    refUs_ = lastPulseUs;
    return rpm_;
  }

  uint16_t rpm() const { return rpm_; }
  uint32_t frequencyMilliHz() const { return freqMilliHz_; }

 private:
  uint8_t ppr_;
  uint32_t timeoutUs_;
  bool haveRef_ = false;
  uint32_t refCount_ = 0;
  uint32_t refUs_ = 0;
  uint16_t rpm_ = 0;
  uint32_t freqMilliHz_ = 0;
};

// Integer exponential moving average. It snaps straight to the raw value when
// the fan stops or starts, so the display never decays slowly towards 0.
inline uint16_t smoothRpm(uint16_t previous, uint16_t raw, uint8_t factor) {
  if (raw == 0 || previous == 0 || factor <= 1) return raw;
  const int32_t delta = (int32_t)raw - (int32_t)previous;
  int32_t next = (int32_t)previous + delta / factor;
  if (next == previous && delta != 0) next += (delta > 0) ? 1 : -1;
  return (uint16_t)next;
}

// Measured speed as a percentage of the maximum, 0..100. A maximum of 0
// (not known) returns 0 instead of dividing by zero; callers show "--".
inline uint8_t rpmToPercent(uint16_t rpm, uint16_t maxRpm) {
  if (maxRpm == 0) return 0;
  const uint32_t pct = ((uint32_t)rpm * 100u + maxRpm / 2u) / maxRpm;
  return pct > 100u ? 100u : (uint8_t)pct;
}

// Fan commanded to run, but no tach pulse for faultMs (counted from the later
// of "fan started" and "last pulse"). Compares ages, not timestamps, so it
// stays correct when millis() wraps after 49.7 days.
inline bool tachFault(uint16_t dutyPermille, uint32_t nowMs,
                      uint32_t lastPulseMs, uint32_t fanStartedMs,
                      uint32_t faultMs) {
  if (dutyPermille == 0) return false;
  const uint32_t sincePulse = nowMs - lastPulseMs;
  const uint32_t sinceStart = nowMs - fanStartedMs;
  return (sincePulse < sinceStart ? sincePulse : sinceStart) > faultMs;
}

// ---------------------------------------------------------------------------
// Controller state machine
// ---------------------------------------------------------------------------
// Owns everything the user can change: ON/OFF, the speed setting, the screen
// mode and the calibration run. Inputs are already-decoded events; the output
// is the duty the fan's speed line should see. Nothing here is persistent:
// a power cycle (or watchdog reset) constructs a fresh Controller, which is
// always OFF with the default setting.
enum class Mode : uint8_t { Normal, Calibrating, CalResult, Diagnostics };

struct ControllerConfig {
  uint8_t settingDefault;   // % at power-up
  uint8_t settingMin;
  uint8_t settingMax;
  uint8_t stepSlow;         // % per detent
  uint8_t stepFast;         // % per detent when spun quickly
  uint32_t fastMs;          // detents closer than this are "fast"
  uint8_t dutyMinPct;       // line duty for a 1 % setting (approx.)
  uint8_t dutyMaxPct;       // line duty for 100 %
  uint16_t maxRpm;          // initial "100 %" speed for the ACT percentage (0 = unknown)
  uint32_t calSpinupMs;
  uint32_t calSampleMs;
  uint16_t calMinValidRpm;
  uint32_t calResultShowMs;
};

class Controller {
 public:
  explicit Controller(const ControllerConfig &c)
      : c_(c), setting_(c.settingDefault), maxRpm_(c.maxRpm) {}

  // Knob held at power-up: open diagnostics (the caller also suppresses the
  // press so it neither toggles the fan nor starts a calibration).
  void begin(bool knobHeldAtBoot) {
    if (knobHeldAtBoot) mode_ = Mode::Diagnostics;
  }

  // Encoder detents (already direction-corrected). Returns true if the
  // display should be redrawn.
  bool onDetents(int8_t detents, uint32_t nowMs) {
    if (detents == 0) return false;
    if (mode_ == Mode::CalResult) {
      mode_ = Mode::Normal;  // any input dismisses the result
    } else if (mode_ == Mode::Normal || mode_ == Mode::Diagnostics) {
      setting_ = applyDetents(setting_, detents, nowMs - lastDetentMs_,
                              c_.fastMs, c_.stepSlow, c_.stepFast,
                              c_.settingMin, c_.settingMax);
    }
    // While calibrating the knob is ignored (press it to cancel instead).
    lastDetentMs_ = nowMs;
    return true;
  }

  bool onKnob(ButtonEvent e, uint32_t nowMs) {
    if (e == ButtonEvent::None) return false;
    switch (mode_) {
      case Mode::Normal:
      case Mode::Diagnostics:
        if (e == ButtonEvent::Short) {
          on_ = !on_;
          return true;
        }
        if (e == ButtonEvent::Long && on_ && mode_ == Mode::Normal) {
          return startCalibration(nowMs);
        }
        return false;
      case Mode::Calibrating:
        // The long press that started calibration produces no Short on
        // release, so only a fresh press cancels.
        if (e == ButtonEvent::Short) return cancelCalibration();
        return false;
      case Mode::CalResult:
        mode_ = Mode::Normal;
        return true;
    }
    return false;
  }

  bool onConKey(ButtonEvent e) {
    if (e != ButtonEvent::Short) return false;
    if (mode_ == Mode::Normal) mode_ = Mode::Diagnostics;
    else if (mode_ == Mode::Diagnostics || mode_ == Mode::CalResult) mode_ = Mode::Normal;
    else return false;
    return true;
  }

  bool onBakKey(ButtonEvent e) {
    if (e != ButtonEvent::Short) return false;
    if (mode_ == Mode::Calibrating) return cancelCalibration();
    if (mode_ == Mode::Diagnostics || mode_ == Mode::CalResult) {
      mode_ = Mode::Normal;
      return true;
    }
    return false;
  }

  // Direct commands (serial console).
  void setOn(bool on) {
    if (!on && mode_ == Mode::Calibrating) mode_ = Mode::Normal;
    on_ = on;
  }
  void setSetting(uint8_t pct) {
    setting_ = (uint8_t)clampI16(pct, c_.settingMin, c_.settingMax);
  }
  void setMaxRpm(uint16_t rpm) { maxRpm_ = rpm; }

  // Starts a calibration run. Only allowed while the fan is ON and on the
  // main screen, so calibration never starts the fan by itself.
  bool startCalibration(uint32_t nowMs) {
    if (!on_ || mode_ != Mode::Normal) return false;
    mode_ = Mode::Calibrating;
    calStartMs_ = nowMs;
    calSum_ = 0;
    calSamples_ = 0;
    calMin_ = 0xFFFF;
    calMax_ = 0;
    return true;
  }

  bool cancelCalibration() {
    if (mode_ != Mode::Calibrating) return false;
    mode_ = Mode::Normal;
    return true;
  }

  // One raw RPM reading per tach window.
  void onTachSample(uint16_t rawRpm, uint32_t nowMs) {
    if (mode_ != Mode::Calibrating) return;
    if (nowMs - calStartMs_ < c_.calSpinupMs) return;
    calSum_ += rawRpm;
    calSamples_++;
    if (rawRpm < calMin_) calMin_ = rawRpm;
    if (rawRpm > calMax_) calMax_ = rawRpm;
  }

  // Time-driven transitions. Returns true if the display should be redrawn.
  bool tick(uint32_t nowMs) {
    if (mode_ == Mode::Calibrating) {
      if (!on_) {
        mode_ = Mode::Normal;
        return true;
      }
      if (nowMs - calStartMs_ >= c_.calSpinupMs + c_.calSampleMs) {
        finishCalibration(nowMs);
        return true;
      }
    } else if (mode_ == Mode::CalResult &&
               nowMs - calResultMs_ >= c_.calResultShowMs) {
      mode_ = Mode::Normal;
      return true;
    }
    return false;
  }

  // Duty the fan's speed line should see, in permille.
  uint16_t dutyPermille() const {
    if (mode_ == Mode::Calibrating && on_) return (uint16_t)(c_.dutyMaxPct * 10u);
    return settingToDutyPermille(setting_, on_, c_.dutyMinPct, c_.dutyMaxPct);
  }

  bool on() const { return on_; }
  uint8_t setting() const { return setting_; }
  Mode mode() const { return mode_; }
  uint16_t maxRpm() const { return maxRpm_; }
  bool calibrated() const { return maxRpm_ != 0; }
  bool lastCalOk() const { return calOk_; }
  uint16_t lastCalRpm() const { return calResultRpm_; }
  uint32_t calElapsedMs(uint32_t nowMs) const { return nowMs - calStartMs_; }

 private:
  void finishCalibration(uint32_t nowMs) {
    calOk_ = false;
    calResultRpm_ = 0;
    if (calSamples_ > 0) {
      const uint16_t avg = (uint16_t)(calSum_ / calSamples_);
      // Accept only a steady reading: spread within +/-10 % of the average.
      const bool steady = (uint32_t)(calMax_ - calMin_) * 10u <= (uint32_t)avg * 2u;
      if (avg >= c_.calMinValidRpm && steady) {
        maxRpm_ = avg;  // RAM only: lost at power-off by design
        calOk_ = true;
        calResultRpm_ = avg;
      }
    }
    mode_ = Mode::CalResult;
    calResultMs_ = nowMs;
  }

  ControllerConfig c_;
  bool on_ = false;  // requirement: OFF at power-up
  uint8_t setting_;  // requirement: kept while powered (RAM only)
  Mode mode_ = Mode::Normal;
  uint16_t maxRpm_;
  uint32_t lastDetentMs_ = 0;
  uint32_t calStartMs_ = 0;
  uint32_t calSum_ = 0;
  uint16_t calSamples_ = 0;
  uint16_t calMin_ = 0xFFFF;
  uint16_t calMax_ = 0;
  bool calOk_ = false;
  uint16_t calResultRpm_ = 0;
  uint32_t calResultMs_ = 0;
};

// ---------------------------------------------------------------------------
// Serial console commands (one per line, case-insensitive)
// ---------------------------------------------------------------------------
//   ?            help              s        status line
//   on / off     fan on / off      set N    speed setting 0..100 %
//   c            start calibration x        cancel calibration
//   m N          set max RPM (RAM only; 0 = unknown)
enum class Cmd : uint8_t { None, Invalid, Help, Status, On, Off, Set, Cal, Cancel, MaxRpm };

struct Command {
  Cmd cmd;
  uint16_t value;
};

inline char lowerAscii(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

// Parses one line (without the line ending). Leading/trailing spaces are
// ignored. Numbers must be 0..65535; "set" is clamped later by the caller.
inline Command parseCommand(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  char word[5];
  uint8_t n = 0;
  while (*s && *s != ' ' && *s != '\t') {
    if (n >= sizeof word - 1) return {Cmd::Invalid, 0};
    word[n++] = lowerAscii(*s++);
  }
  word[n] = 0;
  if (n == 0) return {Cmd::None, 0};
  while (*s == ' ' || *s == '\t') s++;
  bool haveNum = false;
  uint32_t v = 0;
  while (*s >= '0' && *s <= '9') {
    v = v * 10u + (uint32_t)(*s++ - '0');
    if (v > 65535u) return {Cmd::Invalid, 0};
    haveNum = true;
  }
  while (*s == ' ' || *s == '\t') s++;
  if (*s) return {Cmd::Invalid, 0};  // trailing garbage

  struct Entry { const char *name; Cmd cmd; bool needsNum; };
  static const Entry kTable[] = {
      {"?", Cmd::Help, false},  {"help", Cmd::Help, false}, {"s", Cmd::Status, false},
      {"on", Cmd::On, false},   {"off", Cmd::Off, false},   {"set", Cmd::Set, true},
      {"c", Cmd::Cal, false},   {"x", Cmd::Cancel, false},  {"m", Cmd::MaxRpm, true},
  };
  for (const Entry &e : kTable) {
    const char *a = e.name;
    const char *b = word;
    while (*a && *a == *b) { a++; b++; }
    if (*a || *b) continue;
    if (e.needsNum != haveNum) return {Cmd::Invalid, 0};
    return {e.cmd, (uint16_t)v};
  }
  return {Cmd::Invalid, 0};
}

}  // namespace fan
