// fan_logic.h - hardware-independent logic for the Cloudline controller.
//
// Nothing in this file touches Arduino APIs or AVR registers, so the same
// code is compiled into the firmware and into the host unit tests in tests/.
#pragma once

#include <stdint.h>

namespace fan {

// ---------------------------------------------------------------------------
// EC11 quadrature decoder
// ---------------------------------------------------------------------------
// The EC11 used on the AITRIP module has 20 pulses and 20 detents per turn,
// so one detent is one full quadrature cycle (four edges). With pull-ups both
// contacts are open (A=1, B=1) at rest. The decoder accumulates the signed
// Gray-code transitions and emits one step when the shaft comes back to rest,
// so contact bounce (+1 -1 +1 ...) cancels itself out and is never reported.
// Index = (previous << 2) | current, state = A<<1 | B.
// "Positive" order is 11 -> 01 -> 00 -> 10 -> 11 (A falls first).
// Double jumps (invalid) count as 0.
static const int8_t kQuadTransition[16] = {
    0, -1, +1, 0,   // from 00
    +1, 0, 0, -1,   // from 01
    -1, 0, 0, +1,   // from 10
    0, +1, -1, 0,   // from 11
};

class QuadratureDecoder {
 public:
  // Feed the current pin levels; returns +1, -1 or 0 detents.
  int8_t update(bool a, bool b) {
    const uint8_t state = (uint8_t)((a ? 2 : 0) | (b ? 1 : 0));
    if (state == prev_) return 0;
    acc_ += kQuadTransition[(prev_ << 2) | state];
    prev_ = state;
    if (state != kRestState) return 0;
    // A clean detent sums to +/-4. Anything else (bounce that did not make it
    // all the way round, an aborted half turn, or an invalid double jump
    // caused by a missed edge) is discarded rather than guessed at.
    int8_t step = 0;
    if (acc_ >= 4) step = 1;
    else if (acc_ <= -4) step = -1;
    acc_ = 0;
    return step;
  }

  void reset(bool a, bool b) {
    prev_ = (uint8_t)((a ? 2 : 0) | (b ? 1 : 0));
    acc_ = 0;
  }

 private:
  static const uint8_t kRestState = 3;  // A=1, B=1
  uint8_t prev_ = kRestState;
  int8_t acc_ = 0;
};

// ---------------------------------------------------------------------------
// Debounced push button with short-press and long-press events
// ---------------------------------------------------------------------------
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
inline uint16_t settingToDutyPermille(uint8_t settingPct, bool on,
                                      uint8_t dutyMinPct, uint8_t dutyMaxPct) {
  if (!on || settingPct == 0) return 0;
  if (settingPct > 100) settingPct = 100;
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
// (period based) rather than quantised to "pulses per window".
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

// Measured speed as a percentage of the calibrated maximum, 0..100.
inline uint8_t rpmToPercent(uint16_t rpm, uint16_t maxRpm) {
  if (maxRpm == 0) return 0;
  const uint32_t pct = ((uint32_t)rpm * 100u + maxRpm / 2u) / maxRpm;
  return pct > 100u ? 100u : (uint8_t)pct;
}

// ---------------------------------------------------------------------------
// Calibration record kept in EEPROM (the speed setting itself is NOT saved:
// it lives in RAM only, as required, and resets to the default at power-up).
// ---------------------------------------------------------------------------
struct CalRecord {
  uint16_t magic;
  uint16_t maxRpm;
  uint16_t check;
};

const uint16_t kCalMagic = 0xC1A6;

inline CalRecord makeCalRecord(uint16_t maxRpm) {
  return {kCalMagic, maxRpm, (uint16_t)~(kCalMagic ^ maxRpm)};
}

inline bool calRecordValid(const CalRecord &r) {
  return r.magic == kCalMagic && r.check == (uint16_t)~(r.magic ^ r.maxRpm) &&
         r.maxRpm >= 100 && r.maxRpm <= 30000;
}

}  // namespace fan
