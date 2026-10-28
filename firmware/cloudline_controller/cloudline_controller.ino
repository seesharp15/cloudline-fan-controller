// Cloudline AI-CLS6 fan controller
// Arduino Nano (ATmega328P, 16 MHz) + AITRIP 1.3" SH1106 OLED with EC11 encoder.
//
//   Turn the knob ........ change the speed setting (0-100 %, also while OFF)
//   Press the knob ....... fan ON / OFF (always OFF after power-up)
//   Hold the knob 3 s .... (fan ON) calibrate: learn the RPM at 100 %
//   CON key .............. diagnostics page on / off
//   BAK key .............. back / cancel calibration
//   Knob held at power-up  start on the diagnostics page
//
// The speed setting lives in RAM only: it survives ON/OFF toggles and is
// reset to SETTING_DEFAULT_PCT when power is removed. Only the calibration
// (max RPM) is written to EEPROM.
//
// Required library: U8g2 by olikraus (Library Manager: "U8g2").
// Wiring, schematic and verification steps: see the docs/ folder.

#include <Arduino.h>
#include <EEPROM.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "config.h"
#include "display_ui.h"
#include "fan_logic.h"

#if FAN_PWM_FREQ_HZ < 250 || FAN_PWM_FREQ_HZ > 20000
#error "FAN_PWM_FREQ_HZ must be between 250 and 20000"
#endif
#if TACH_FAULT_MS < TACH_TIMEOUT_MS
#error "TACH_FAULT_MS must not be shorter than TACH_TIMEOUT_MS"
#endif

#if OLED_SSD1306 && OLED_PAGE_BUFFER
U8G2_SSD1306_128X64_NONAME_2_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#elif OLED_SSD1306
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#elif OLED_PAGE_BUFFER
U8G2_SH1106_128X64_NONAME_2_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#else
// Full-frame buffer (1 KB of RAM) so a frame is drawn once and sent in one go.
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#endif

// ---------------------------------------------------------------------------
// State shared with interrupt handlers
// ---------------------------------------------------------------------------
volatile uint32_t gTachPulses = 0;
volatile uint32_t gTachLastUs = 0;
volatile int8_t gEncDetents = 0;
fan::QuadratureDecoder gDecoder;  // only touched inside the PCINT2 ISR after setup

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------
enum class Mode : uint8_t { Normal, Calibrating, CalResult, Diagnostics };

Mode gMode = Mode::Normal;
bool gFanOn = false;                     // requirement: OFF at power-up
uint8_t gSetting = SETTING_DEFAULT_PCT;  // requirement: kept while powered
uint16_t gMaxRpm = TACH_DEFAULT_MAX_RPM;
bool gCalibrated = false;

fan::Button gKnob(BUTTON_DEBOUNCE_MS, CAL_HOLD_MS);
fan::Button gKeyCon(BUTTON_DEBOUNCE_MS, 0xFFFFFFFFUL);
fan::Button gKeyBak(BUTTON_DEBOUNCE_MS, 0xFFFFFFFFUL);
uint32_t gLastDetentMs = 0;

fan::TachEstimator gTach(TACH_PULSES_PER_REV, TACH_TIMEOUT_MS * 1000UL);
uint16_t gRpm = 0;               // smoothed, for display
uint16_t gRawRpm = 0;
uint32_t gLastTachCalcMs = 0;
uint32_t gLastPulseCount = 0;
uint32_t gLastPulseSeenMs = 0;
uint32_t gFanStartedMs = 0;      // when the fan line last went from 0 to > 0 duty

uint16_t gPwmTop = 0;
uint16_t gDutyPermille = 0xFFFF;  // forces the first applyDuty()
fan::GateDrive gGate = {fan::GateDrive::ConstantHigh, 0};

// Calibration run
uint32_t gCalStartMs = 0;
uint32_t gCalSum = 0;
uint16_t gCalSamples = 0;
uint16_t gCalMin = 0xFFFF;
uint16_t gCalMax = 0;
bool gCalOk = false;
uint16_t gCalResultRpm = 0;
uint32_t gCalResultMs = 0;
const uint32_t kCalResultShowMs = 3000;

bool gDirty = true;
uint32_t gLastDrawMs = 0;

// ---------------------------------------------------------------------------
// Interrupt handlers
// ---------------------------------------------------------------------------
// Tach pulse (D2 falls when the fan's tach output goes high, Q2 inverts).
void onTachPulse() {
  const uint32_t now = micros();
  if ((uint32_t)(now - gTachLastUs) < TACH_MIN_PERIOD_US) return;  // glitch
  gTachLastUs = now;
  gTachPulses++;
}

// Any edge on D3 (TRA) or D4 (TRB).
ISR(PCINT2_vect) {
  const int8_t step = gDecoder.update(digitalRead(PIN_ENC_A), digitalRead(PIN_ENC_B));
  if (step > 0 && gEncDetents < 100) gEncDetents++;
  else if (step < 0 && gEncDetents > -100) gEncDetents--;
}

// ---------------------------------------------------------------------------
// Fan PWM on D9 / OC1A (Timer1, fast PWM mode 14, TOP = ICR1, no prescaler)
// ---------------------------------------------------------------------------
void applyGate(const fan::GateDrive &g) {
  switch (g.mode) {
    case fan::GateDrive::ConstantLow:
      TCCR1A &= (uint8_t)~(_BV(COM1A1) | _BV(COM1A0));  // pin back to PORT control
      digitalWrite(PIN_FAN_PWM, LOW);
      break;
    case fan::GateDrive::ConstantHigh:
      TCCR1A &= (uint8_t)~(_BV(COM1A1) | _BV(COM1A0));
      digitalWrite(PIN_FAN_PWM, HIGH);
      break;
    case fan::GateDrive::Pwm:
      OCR1A = g.compare;
      TCCR1A |= _BV(COM1A1);  // non-inverting: HIGH from BOTTOM to compare match
      break;
  }
  gGate = g;
}

void setupFanPwm() {
  // Drive the gate HIGH before the pin becomes an output, so Q1 holds the fan
  // line LOW (= fan stopped) without a glitch. R4 already does this while the
  // Nano is in reset or running the bootloader.
  digitalWrite(PIN_FAN_PWM, FAN_PWM_INVERTED ? HIGH : LOW);
  pinMode(PIN_FAN_PWM, OUTPUT);

  gPwmTop = fan::timer1TopFor(F_CPU, 1, FAN_PWM_FREQ_HZ);
  TCCR1B = 0;  // stop the timer while configuring
  TCCR1A = _BV(WGM11);
  ICR1 = gPwmTop;
  OCR1A = 0;
  TCNT1 = 0;
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
}

void applyDuty(uint16_t dutyPermille) {
  if (dutyPermille == gDutyPermille) return;
  if (gDutyPermille == 0 || gDutyPermille == 0xFFFF) {
    if (dutyPermille > 0) gFanStartedMs = millis();
  }
  gDutyPermille = dutyPermille;
  applyGate(fan::gateDriveFor(dutyPermille, gPwmTop, FAN_PWM_INVERTED));
}

// ---------------------------------------------------------------------------
// Calibration storage
// ---------------------------------------------------------------------------
void loadCalibration() {
  fan::CalRecord r;
  EEPROM.get(0, r);
  if (fan::calRecordValid(r)) {
    gMaxRpm = r.maxRpm;
    gCalibrated = true;
  }
}

void saveCalibration(uint16_t maxRpm) {
  const fan::CalRecord r = fan::makeCalRecord(maxRpm);
  EEPROM.put(0, r);  // put() only rewrites bytes that changed
  gMaxRpm = maxRpm;
  gCalibrated = true;
}

void startCalibration(uint32_t now) {
  gMode = Mode::Calibrating;
  gCalStartMs = now;
  gCalSum = 0;
  gCalSamples = 0;
  gCalMin = 0xFFFF;
  gCalMax = 0;
}

void finishCalibration(uint32_t now) {
  gCalOk = false;
  gCalResultRpm = 0;
  if (gCalSamples > 0) {
    const uint16_t avg = (uint16_t)(gCalSum / gCalSamples);
    // Accept only a steady reading: spread within +/-10 % of the average.
    const bool steady = (uint32_t)(gCalMax - gCalMin) * 10u <= (uint32_t)avg * 2u;
    if (avg >= CAL_MIN_VALID_RPM && steady) {
      saveCalibration(avg);
      gCalOk = true;
      gCalResultRpm = avg;
    }
  }
  gMode = Mode::CalResult;
  gCalResultMs = now;
}

// Called once per tach window while calibrating.
void calibrationSample(uint32_t now) {
  if (now - gCalStartMs < CAL_SPINUP_MS) return;
  gCalSum += gRawRpm;
  gCalSamples++;
  if (gRawRpm < gCalMin) gCalMin = gRawRpm;
  if (gRawRpm > gCalMax) gCalMax = gRawRpm;
}

// ---------------------------------------------------------------------------
// Inputs
// ---------------------------------------------------------------------------
bool pressed(uint8_t pin) { return digitalRead(pin) == LOW; }  // keys switch to GND

void handleEncoder(uint32_t now) {
  noInterrupts();
  int8_t detents = gEncDetents;
  gEncDetents = 0;
  interrupts();
  if (detents == 0) return;
  if (ENC_REVERSE) detents = (int8_t)-detents;

  if (gMode == Mode::CalResult) {
    gMode = Mode::Normal;  // any input dismisses the result
  } else if (gMode == Mode::Normal || gMode == Mode::Diagnostics) {
    gSetting = fan::applyDetents(gSetting, detents, now - gLastDetentMs, ENC_FAST_MS,
                                 SETTING_STEP_PCT, SETTING_FAST_STEP_PCT,
                                 SETTING_MIN_PCT, SETTING_MAX_PCT);
  }
  // While calibrating the knob is ignored (press it to cancel instead).
  gLastDetentMs = now;
  gDirty = true;
}

void handleButtons(uint32_t now) {
  const fan::ButtonEvent knob = gKnob.update(pressed(PIN_ENC_SW), now);
  const fan::ButtonEvent con = gKeyCon.update(pressed(PIN_KEY_CON), now);
  const fan::ButtonEvent bak = gKeyBak.update(pressed(PIN_KEY_BAK), now);
  if (gKnob.held()) gDirty = true;  // animate the "Hold for CAL" countdown

  switch (gMode) {
    case Mode::Normal:
    case Mode::Diagnostics:
      if (knob == fan::ButtonEvent::Short) {
        gFanOn = !gFanOn;
        gDirty = true;
      } else if (knob == fan::ButtonEvent::Long && gFanOn && gMode == Mode::Normal) {
        startCalibration(now);
        gDirty = true;
      }
      if (con == fan::ButtonEvent::Short) {
        gMode = (gMode == Mode::Normal) ? Mode::Diagnostics : Mode::Normal;
        gDirty = true;
      }
      if (bak == fan::ButtonEvent::Short && gMode == Mode::Diagnostics) {
        gMode = Mode::Normal;
        gDirty = true;
      }
      break;

    case Mode::Calibrating:
      // The long press that started calibration does not produce a Short on
      // release, so a fresh press of the knob (or BAK) cancels.
      if (knob == fan::ButtonEvent::Short || bak == fan::ButtonEvent::Short) {
        gMode = Mode::Normal;
        gDirty = true;
      }
      break;

    case Mode::CalResult:
      if (knob != fan::ButtonEvent::None || con != fan::ButtonEvent::None ||
          bak != fan::ButtonEvent::None) {
        gMode = Mode::Normal;
        gDirty = true;
      }
      break;
  }
}

// ---------------------------------------------------------------------------
// Tachometer
// ---------------------------------------------------------------------------
void updateTach(uint32_t now) {
  if (now - gLastTachCalcMs < TACH_WINDOW_MS) return;
  gLastTachCalcMs = now;

  noInterrupts();
  const uint32_t count = gTachPulses;
  const uint32_t lastUs = gTachLastUs;
  interrupts();

  if (count != gLastPulseCount) {
    gLastPulseCount = count;
    gLastPulseSeenMs = now;
  }
  gRawRpm = gTach.update(count, lastUs, micros());
  const uint16_t smoothed = fan::smoothRpm(gRpm, gRawRpm, TACH_SMOOTHING);
  if (smoothed != gRpm) gDirty = true;
  gRpm = smoothed;

  if (gMode == Mode::Calibrating) calibrationSample(now);
}

// Fan commanded to run, but no tach pulse for TACH_FAULT_MS.
bool tachFault(uint32_t now) {
  if (gDutyPermille == 0 || gDutyPermille == 0xFFFF) return false;
  // Compare ages, not timestamps, so this stays correct when millis() wraps.
  const uint32_t sincePulse = now - gLastPulseSeenMs;
  const uint32_t sinceStart = now - gFanStartedMs;
  return (sincePulse < sinceStart ? sincePulse : sinceStart) > TACH_FAULT_MS;
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
void updateDisplay(uint32_t now) {
  const uint32_t since = now - gLastDrawMs;
  if (!(gDirty && since >= DISPLAY_MIN_GAP_MS) && since < DISPLAY_REFRESH_MS) return;
  gDirty = false;
  gLastDrawMs = now;

  ui::State s;
  s.fanOn = gFanOn;
  s.settingPct = gSetting;
  s.rpm = gRpm;
  s.actualPct = fan::rpmToPercent(gRpm, gMaxRpm);
  s.tachFault = tachFault(now);
  s.calibrated = gCalibrated;
  s.knobHeldMs = (gMode == Mode::Normal) ? gKnob.heldMs(now) : 0;
  s.calHoldMs = CAL_HOLD_MS;

  switch (gMode) {
    case Mode::Normal:
      s.screen = ui::Screen::Main;
      break;
    case Mode::Calibrating: {
      s.screen = ui::Screen::Calibrating;
      const uint32_t el = now - gCalStartMs;
      const uint32_t total = CAL_SPINUP_MS + CAL_SAMPLE_MS;
      s.calSampling = el >= CAL_SPINUP_MS;
      const uint32_t phaseLeft = s.calSampling ? (total > el ? total - el : 0)
                                               : CAL_SPINUP_MS - el;
      s.calSecondsLeft = (uint8_t)((phaseLeft + 999) / 1000);
      s.calProgressPct = (uint8_t)(el >= total ? 100 : el * 100 / total);
      s.calRpm = gRpm;
      break;
    }
    case Mode::CalResult:
      s.screen = ui::Screen::CalResult;
      s.calOk = gCalOk;
      s.calRpm = gCalResultRpm;
      break;
    case Mode::Diagnostics:
      s.screen = ui::Screen::Diagnostics;
      break;
  }
  s.tachMilliHz = gTach.frequencyMilliHz();
  noInterrupts();
  s.tachPulses = gTachPulses;
  interrupts();
  s.pulsesPerRev = TACH_PULSES_PER_REV;
  s.dutyPermille = gDutyPermille;
  s.maxRpm = gMaxRpm;
  s.pwmCompare = gGate.compare;
  s.pwmTop = gPwmTop;
  s.gateMode = (uint8_t)gGate.mode;

#if OLED_PAGE_BUFFER
  oled.firstPage();
  do {
    ui::draw(oled, s);
  } while (oled.nextPage());
#else
  oled.clearBuffer();
  ui::draw(oled, s);
  oled.sendBuffer();
#endif
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------
void setup() {
  setupFanPwm();  // first: guarantees the fan is stopped
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Keys switch to GND. Internal pull-ups are used whether or not the module
  // has its own (if it does, they go to 3.3 V and the two simply share).
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_KEY_CON, INPUT_PULLUP);
  pinMode(PIN_KEY_BAK, INPUT_PULLUP);
  // The tach buffer (Q2) has its own 10 k pull-up to 5 V (R9).
  pinMode(PIN_TACH, INPUT);

  loadCalibration();

  oled.setI2CAddress(OLED_I2C_ADDRESS << 1);  // U8g2 expects the 8-bit address
  oled.setBusClock(OLED_I2C_CLOCK_HZ);
  oled.begin();
  // Wire.begin() enabled the AVR's internal pull-ups to 5 V on SDA/SCL. The
  // SH1106 runs at 3.3 V, so turn them off and rely on R1/R2 (to 3.3 V).
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);

  // Knob held during power-up: open diagnostics, and ignore that press so it
  // neither toggles the fan nor starts a calibration.
  delay(BUTTON_DEBOUNCE_MS + 5);
  const uint32_t now = millis();
  gKnob.update(pressed(PIN_ENC_SW), now);
  if (pressed(PIN_ENC_SW)) {
    gKnob.suppressUntilRelease();
    gMode = Mode::Diagnostics;
  }

  // Encoder: pin-change interrupts on D3/D4 (both on PCINT2 / port D).
  gDecoder.reset(digitalRead(PIN_ENC_A), digitalRead(PIN_ENC_B));
  *digitalPinToPCMSK(PIN_ENC_A) |= _BV(digitalPinToPCMSKbit(PIN_ENC_A));
  *digitalPinToPCMSK(PIN_ENC_B) |= _BV(digitalPinToPCMSKbit(PIN_ENC_B));
  PCIFR = _BV(digitalPinToPCICRbit(PIN_ENC_A));  // clear any stale flag
  PCICR |= _BV(digitalPinToPCICRbit(PIN_ENC_A));

  attachInterrupt(digitalPinToInterrupt(PIN_TACH), onTachPulse, FALLING);

  applyDuty(0);
}

void loop() {
  const uint32_t now = millis();

  handleEncoder(now);
  handleButtons(now);
  updateTach(now);

  if (gMode == Mode::Calibrating) {
    if (!gFanOn) gMode = Mode::Normal;  // cannot happen via the UI, but be safe
    else if (now - gCalStartMs >= CAL_SPINUP_MS + CAL_SAMPLE_MS) finishCalibration(now);
  } else if (gMode == Mode::CalResult && now - gCalResultMs >= kCalResultShowMs) {
    gMode = Mode::Normal;
    gDirty = true;
  }

  const uint16_t duty = (gMode == Mode::Calibrating)
                            ? (uint16_t)(FAN_DUTY_MAX_PCT * 10u)
                            : fan::settingToDutyPermille(gSetting, gFanOn, FAN_DUTY_MIN_PCT,
                                                         FAN_DUTY_MAX_PCT);
  applyDuty(duty);
  digitalWrite(PIN_STATUS_LED, gFanOn ? HIGH : LOW);

  updateDisplay(now);
}
