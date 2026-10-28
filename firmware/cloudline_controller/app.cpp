// app.cpp - hardware glue for the Cloudline AI-CLS6 controller.
//
// Pins, Timer1 PWM, interrupts, watchdog, display and serial console. All
// decisions (ON/OFF, setting, calibration, duty) are made by fan::Controller
// in fan_logic.h, which is unit-tested on the host.
//
// Interrupt allocation (the Nano has only two external interrupts, D2/D3):
//   INT0  (D2)      tach pulses: needs precise timing of single edges.
//   PCINT2 (D3, D4) encoder A/B: one pin-change vector covers both pins.
//   polled          encoder push switch and CON/BAK keys (debounced in loop).
// D3's own INT1 is not used; the pin-change interrupt sees its edges.

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <avr/wdt.h>

#include "config.h"
#include "display_ui.h"
#include "fan_logic.h"

#if FAN_PWM_FREQ_HZ < 250 || FAN_PWM_FREQ_HZ > 20000
#error "FAN_PWM_FREQ_HZ must be between 250 and 20000"
#endif
#if TACH_FAULT_MS < TACH_TIMEOUT_MS
#error "TACH_FAULT_MS must not be shorter than TACH_TIMEOUT_MS"
#endif
#if FAN_DUTY_MIN_PCT > FAN_DUTY_MAX_PCT || FAN_DUTY_MAX_PCT > 100
#error "FAN_DUTY_MIN_PCT/FAN_DUTY_MAX_PCT must satisfy 0 <= MIN <= MAX <= 100"
#endif
#if PIN_ENC_A > 7 || PIN_ENC_B > 7
#error "The encoder pins must both be on port D (D0..D7) for PCINT2"
#endif
#if PIN_TACH != 2
#error "The tach must be on D2 (INT0)"
#endif
#if PIN_FAN_PWM != 9
#error "The fan PWM must be on D9 (OC1A, Timer1)"
#endif

#if OLED_SSD1306 && OLED_PAGE_BUFFER
U8G2_SSD1306_128X64_NONAME_2_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#elif OLED_SSD1306
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#elif OLED_PAGE_BUFFER
U8G2_SH1106_128X64_NONAME_2_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#else
// Full frame buffer (1 KB of RAM): each frame is drawn off-screen and sent
// in one go, so the panel never shows a half-drawn or cleared frame.
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#endif

// ---------------------------------------------------------------------------
// Reset cause, watchdog
// ---------------------------------------------------------------------------
// gResetFlags: MCUSR as the sketch found it (0 if the bootloader cleared it),
// plus kSoftRestartFlag when the watchdog interrupt restarted the sketch.
// Both live in .noinit so the C runtime start-up does not clear them.
const uint8_t kSoftRestartFlag = 0x80;
const uint8_t kSoftRestartMagic = 0xA5;
#ifdef __AVR__
#define NOINIT __attribute__((section(".noinit")))
#else
#define NOINIT
#endif
uint8_t gResetFlags NOINIT;
uint8_t gSoftRestart NOINIT;

#ifdef __AVR__
// Runs before RAM is initialised. After a hardware watchdog reset the
// watchdog stays enabled with its shortest timeout (16 ms), so turn it off
// first; setup() re-enables it with the configured timeout.
extern "C" void earlyResetHandling(void) __attribute__((naked, used, section(".init3")));
extern "C" void earlyResetHandling(void) {
  gResetFlags = MCUSR;
  MCUSR = 0;
  wdt_disable();
  // gSoftRestart is random after power-up, so only trust it when no
  // hardware reset cause is recorded.
  const uint8_t hw = _BV(PORF) | _BV(EXTRF) | _BV(BORF) | _BV(WDRF);
  if (gSoftRestart == kSoftRestartMagic && !(gResetFlags & hw)) gResetFlags |= kSoftRestartFlag;
  gSoftRestart = 0;
}

// "Interrupt and system reset" mode: the first timeout runs WDT_vect (and
// the hardware clears WDIE); only if that cannot run does a second timeout
// reset the chip.
void watchdogStart() {
  const uint8_t v = WATCHDOG_TIMEOUT;
  const uint8_t prescaler = (uint8_t)((v & 7) | ((v & 8) ? _BV(WDP3) : 0));
  noInterrupts();
  wdt_reset();
  WDTCSR = _BV(WDCE) | _BV(WDE);
  WDTCSR = (uint8_t)(_BV(WDIE) | _BV(WDE) | prescaler);
  interrupts();
}

// loop() has not run for WATCHDOG_TIMEOUT. Stop the fan at once, then
// restart the sketch at address 0. Jumping to 0 skips the bootloader, so the
// old Nano bootloader's watchdog reset loop cannot happen; setup()
// re-initialises every peripheral the sketch uses.
ISR(WDT_vect) {
  TCCR1A = 0;  // disconnect OC1A from D9
#if FAN_PWM_INVERTED
  PORTB |= _BV(PORTB1);  // D9 HIGH: Q1 on, fan line low, fan stopped
#else
  PORTB &= (uint8_t)~_BV(PORTB1);
#endif
  gSoftRestart = kSoftRestartMagic;
  asm volatile("jmp 0");
}
#else
// Host build (test/): the simulation records that the watchdog was started.
bool gWatchdogStarted = false;
void watchdogStart() { gWatchdogStarted = true; }
#endif

// ---------------------------------------------------------------------------
// Stack headroom: setup() paints the free RAM between the end of the static
// data and the stack; stackFreeMin() counts how much paint is left, i.e. the
// least free RAM there has been since power-up. Reported by the "s" command.
// ---------------------------------------------------------------------------
#ifdef __AVR__
extern uint8_t __heap_start;
const uint8_t kStackPaint = 0xC5;
void stackPaint() {
  uint8_t marker;
  for (uint8_t *p = &__heap_start; p < &marker - 32; p++) *p = kStackPaint;
}
uint16_t stackFreeMin() {
  const uint8_t *p = &__heap_start;
  uint16_t n = 0;
  while (*p == kStackPaint && p < (const uint8_t *)RAMEND) { p++; n++; }
  return n;
}
#else
void stackPaint() {}
uint16_t stackFreeMin() { return 0; }
#endif

// ---------------------------------------------------------------------------
// State shared with interrupt handlers
// ---------------------------------------------------------------------------
volatile uint32_t gTachPulses = 0;
volatile uint32_t gTachLastUs = 0;
volatile int8_t gEncDetents = 0;
fan::QuadratureDecoder gDecoder(ENC_EDGES_PER_DETENT);  // ISR-only after setup

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------
const fan::ControllerConfig kControllerConfig = {
    SETTING_DEFAULT_PCT, SETTING_MIN_PCT, SETTING_MAX_PCT,
    SETTING_STEP_PCT,    SETTING_FAST_STEP_PCT, ENC_FAST_MS,
    FAN_DUTY_MIN_PCT,    FAN_DUTY_MAX_PCT,
    TACH_MAX_RPM,
    CAL_SPINUP_MS,       CAL_SAMPLE_MS, CAL_MIN_VALID_RPM, CAL_RESULT_SHOW_MS,
};
fan::Controller gCtl(kControllerConfig);

fan::Button gKnob(BUTTON_DEBOUNCE_MS, CAL_HOLD_MS);
fan::Button gKeyCon(BUTTON_DEBOUNCE_MS, 0xFFFFFFFFUL);
fan::Button gKeyBak(BUTTON_DEBOUNCE_MS, 0xFFFFFFFFUL);

fan::TachEstimator gTach(TACH_PULSES_PER_REV, TACH_TIMEOUT_MS * 1000UL);
uint16_t gRpm = 0;               // smoothed, for display
uint16_t gRawRpm = 0;
uint32_t gLastTachCalcMs = 0;
uint32_t gLastPulseCount = 0;
uint32_t gLastPulseSeenMs = 0;
uint32_t gFanStartedMs = 0;      // when the line duty last went from 0 to > 0

uint16_t gPwmTop = 0;
uint16_t gDutyPermille = 0xFFFF;  // forces the first applyDuty()
fan::GateDrive gGate = {fan::GateDrive::ConstantHigh, 0};

fan::Mode gLastMode = fan::Mode::Normal;
bool gDirty = true;
uint32_t gLastDrawMs = 0;

char gLine[24];
uint8_t gLineLen = 0;

// ---------------------------------------------------------------------------
// Interrupt handlers
// ---------------------------------------------------------------------------
// Tach pulse: D2 falls when the fan's tach output pulls the line low.
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
  // Drive the gate to the "fan stopped" level before the pin becomes an
  // output, so there is no glitch. R4 already holds that level while the
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

#if SIM_FAKE_TACH
// Wokwi has no fan: emit a square wave proportional to the line duty on
// PIN_SIM_TACH_OUT (wired to D2 in sim/diagram.json). tone() uses Timer2 and
// cannot go below 31 Hz, so very low speeds read as stopped.
void updateFakeTach(uint16_t dutyPermille) {
  const uint32_t maxRpm = TACH_MAX_RPM ? TACH_MAX_RPM : 2000;
  const uint32_t hz = maxRpm * dutyPermille / 1000u * TACH_PULSES_PER_REV / 60u;
  if (hz >= 31) tone(PIN_SIM_TACH_OUT, (unsigned int)hz);
  else noTone(PIN_SIM_TACH_OUT);
}
#endif

void applyDuty(uint16_t dutyPermille) {
  if (dutyPermille == gDutyPermille) return;
  if ((gDutyPermille == 0 || gDutyPermille == 0xFFFF) && dutyPermille > 0) {
    gFanStartedMs = millis();
  }
  gDutyPermille = dutyPermille;
  applyGate(fan::gateDriveFor(dutyPermille, gPwmTop, FAN_PWM_INVERTED));
#if SIM_FAKE_TACH
  updateFakeTach(dutyPermille);
#endif
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
  if (ENC_REVERSE) detents = (int8_t)-detents;
  if (gCtl.onDetents(detents, now)) gDirty = true;
}

void handleButtons(uint32_t now) {
  const fan::ButtonEvent knob = gKnob.update(pressed(PIN_ENC_SW), now);
  const fan::ButtonEvent con = gKeyCon.update(pressed(PIN_KEY_CON), now);
  const fan::ButtonEvent bak = gKeyBak.update(pressed(PIN_KEY_BAK), now);
  if (gKnob.held()) gDirty = true;  // animate the "Hold for CAL" countdown
  if (gCtl.onKnob(knob, now)) gDirty = true;
  if (gCtl.onConKey(con)) gDirty = true;
  if (gCtl.onBakKey(bak)) gDirty = true;
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
  gCtl.onTachSample(gRawRpm, now);
}

bool tachFaultNow(uint32_t now) {
  if (gDutyPermille == 0xFFFF) return false;
  return fan::tachFault(gDutyPermille, now, gLastPulseSeenMs, gFanStartedMs, TACH_FAULT_MS);
}

// ---------------------------------------------------------------------------
// Serial console (115200 baud, newline-terminated commands, see fan_logic.h)
// ---------------------------------------------------------------------------
void printStatus(uint32_t now) {
  Serial.print(F("on="));
  Serial.print(gCtl.on() ? 1 : 0);
  Serial.print(F(" set="));
  Serial.print(gCtl.setting());
  Serial.print(F("% duty="));
  Serial.print(gDutyPermille / 10);
  Serial.print('.');
  Serial.print(gDutyPermille % 10);
  Serial.print(F("% rpm="));
  Serial.print(gRpm);
  Serial.print(F(" act="));
  if (gCtl.maxRpm()) {
    Serial.print(fan::rpmToPercent(gRpm, gCtl.maxRpm()));
    Serial.print('%');
  } else {
    Serial.print(F("--"));
  }
  Serial.print(F(" max="));
  Serial.print(gCtl.maxRpm());
  if (!gCtl.calibrated()) Serial.print(F("(unknown)"));
  Serial.print(F(" tach_mHz="));
  Serial.print(gTach.frequencyMilliHz());
  Serial.print(F(" fault="));
  Serial.print(tachFaultNow(now) ? 1 : 0);
  Serial.print(F(" mode="));
  Serial.print((uint8_t)gCtl.mode());
  Serial.print(F(" ram_free_min="));
  Serial.print(stackFreeMin());
  Serial.print(F(" reset=0x"));
  Serial.println(gResetFlags, HEX);
}

void printHelp() {
  Serial.println(F("Commands: ? | s | on | off | set N (0-100) | c (calibrate, fan must be on)"));
  Serial.println(F("          x (cancel calibration) | m N (max RPM, RAM only, 0 = unknown)"));
}

void runCommand(const char *line, uint32_t now) {
  const fan::Command c = fan::parseCommand(line);
  switch (c.cmd) {
    case fan::Cmd::None: return;
    case fan::Cmd::Invalid: Serial.println(F("? unknown command, send ? for help")); return;
    case fan::Cmd::Help: printHelp(); return;
    case fan::Cmd::Status: break;
    case fan::Cmd::On: gCtl.setOn(true); break;
    case fan::Cmd::Off: gCtl.setOn(false); break;
    case fan::Cmd::Set: gCtl.setSetting(c.value > 255 ? 255 : (uint8_t)c.value); break;
    case fan::Cmd::Cal:
      if (!gCtl.startCalibration(now)) {
        Serial.println(F("calibration needs the fan ON and the main screen"));
        return;
      }
      Serial.println(F("calibrating: 100% for spin-up, then measuring"));
      break;
    case fan::Cmd::Cancel: gCtl.cancelCalibration(); break;
    case fan::Cmd::MaxRpm: gCtl.setMaxRpm(c.value); break;
  }
  gDirty = true;
  printStatus(now);
}

void pollSerial(uint32_t now) {
  // Bounded per loop so a flood of input cannot starve the watchdog.
  for (uint8_t i = 0; i < 16 && Serial.available() > 0; i++) {
    const char ch = (char)Serial.read();
    if (ch == '\r' || ch == '\n') {
      gLine[gLineLen] = 0;
      runCommand(gLine, now);
      gLineLen = 0;
    } else if (gLineLen < sizeof gLine - 1) {
      gLine[gLineLen++] = ch;
    }
  }
}

// Report the end of a calibration run once.
void reportModeChange() {
  const fan::Mode m = gCtl.mode();
  if (m == gLastMode) return;
  if (m == fan::Mode::CalResult) {
    if (gCtl.lastCalOk()) {
      Serial.print(F("CAL OK: 100% = "));
      Serial.print(gCtl.lastCalRpm());
      Serial.println(F(" RPM (kept until power-off)."));
      Serial.print(F("To keep it after power-off, set in config.h:  #define TACH_MAX_RPM "));
      Serial.println(gCtl.lastCalRpm());
    } else {
      Serial.println(F("CAL FAILED: no steady tach signal at 100%"));
    }
  }
  gLastMode = m;
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
  s.fanOn = gCtl.on();
  s.settingPct = gCtl.setting();
  s.rpm = gRpm;
  s.actualPct = fan::rpmToPercent(gRpm, gCtl.maxRpm());
  s.tachFault = tachFaultNow(now);
  s.calibrated = gCtl.calibrated();
  s.knobHeldMs = (gCtl.mode() == fan::Mode::Normal) ? gKnob.heldMs(now) : 0;
  s.calHoldMs = CAL_HOLD_MS;

  switch (gCtl.mode()) {
    case fan::Mode::Normal:
      s.screen = ui::Screen::Main;
      break;
    case fan::Mode::Calibrating: {
      s.screen = ui::Screen::Calibrating;
      const uint32_t el = gCtl.calElapsedMs(now);
      const uint32_t total = CAL_SPINUP_MS + CAL_SAMPLE_MS;
      s.calSampling = el >= CAL_SPINUP_MS;
      const uint32_t phaseLeft = s.calSampling ? (total > el ? total - el : 0)
                                               : CAL_SPINUP_MS - el;
      s.calSecondsLeft = (uint8_t)((phaseLeft + 999) / 1000);
      s.calProgressPct = (uint8_t)(el >= total ? 100 : el * 100 / total);
      s.calRpm = gRpm;
      break;
    }
    case fan::Mode::CalResult:
      s.screen = ui::Screen::CalResult;
      s.calOk = gCtl.lastCalOk();
      s.calRpm = gCtl.lastCalRpm();
      break;
    case fan::Mode::Diagnostics:
      s.screen = ui::Screen::Diagnostics;
      break;
  }
  s.tachMilliHz = gTach.frequencyMilliHz();
  noInterrupts();
  s.tachPulses = gTachPulses;
  interrupts();
  s.pulsesPerRev = TACH_PULSES_PER_REV;
  s.dutyPermille = gDutyPermille;
  s.maxRpm = gCtl.maxRpm();
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
  stackPaint();
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Keys switch to GND. Internal pull-ups (to 5 V) are used whether or not
  // the module has its own; the encoder contacts do not connect to the
  // SH1106, so 5 V on them is harmless.
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_KEY_CON, INPUT_PULLUP);
  pinMode(PIN_KEY_BAK, INPUT_PULLUP);
  // The tach input has its own 10 k pull-up to 5 V (R6) and clamp (D3).
  pinMode(PIN_TACH, INPUT);

  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Cloudline controller ready. Fan OFF. Send ? for help."));
  if (gResetFlags & kSoftRestartFlag) Serial.println(F("! restarted by the watchdog"));

  oled.setI2CAddress(OLED_I2C_ADDRESS << 1);  // U8g2 expects the 8-bit address
  oled.setBusClock(OLED_I2C_CLOCK_HZ);
  oled.begin();
  // Wire.begin() enabled the AVR's internal pull-ups to 5 V on SDA/SCL. The
  // OLED runs at 3.3 V, so turn them off and rely on R1/R2 (to 3.3 V).
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);

  // Knob held during power-up: open diagnostics, and ignore that press so it
  // neither toggles the fan nor starts a calibration.
  delay(BUTTON_DEBOUNCE_MS + 5);
  const uint32_t now = millis();
  const bool heldAtBoot = pressed(PIN_ENC_SW);
  gKnob.update(heldAtBoot, now);
  if (heldAtBoot) gKnob.suppressUntilRelease();
  gCtl.begin(heldAtBoot);
  gLastMode = gCtl.mode();

  // Encoder: pin-change interrupts on D3/D4 (both on PCINT2 / port D).
  gDecoder.reset(digitalRead(PIN_ENC_A), digitalRead(PIN_ENC_B));
  *digitalPinToPCMSK(PIN_ENC_A) |= _BV(digitalPinToPCMSKbit(PIN_ENC_A));
  *digitalPinToPCMSK(PIN_ENC_B) |= _BV(digitalPinToPCMSKbit(PIN_ENC_B));
  PCIFR = _BV(digitalPinToPCICRbit(PIN_ENC_A));  // clear any stale flag
  PCICR |= _BV(digitalPinToPCICRbit(PIN_ENC_A));

  attachInterrupt(digitalPinToInterrupt(PIN_TACH), onTachPulse, FALLING);

  applyDuty(gCtl.dutyPermille());  // 0: the controller always starts OFF

#if WATCHDOG_ENABLE
  watchdogStart();
#endif
}

void loop() {
#if WATCHDOG_ENABLE
  wdt_reset();
#endif
  const uint32_t now = millis();

  handleEncoder(now);
  handleButtons(now);
  pollSerial(now);
  updateTach(now);
  if (gCtl.tick(now)) gDirty = true;
  reportModeChange();

  applyDuty(gCtl.dutyPermille());
  digitalWrite(PIN_STATUS_LED, gCtl.on() ? HIGH : LOW);

  updateDisplay(now);

  // Hooks for the AVR simulation test (test/avr/run_simavr.sh). They are
  // compiled in only when the test passes -D flags; normal builds skip them.
#ifdef TEST_STATUS_EVERY_MS
  static uint32_t lastStatus = 0;
  if (now - lastStatus >= TEST_STATUS_EVERY_MS) {
    lastStatus = now;
    printStatus(now);
  }
#endif
#ifdef TEST_HANG_AFTER_MS
  if (!(gResetFlags & kSoftRestartFlag) && now > TEST_HANG_AFTER_MS) {
    Serial.println(F("TEST: loop() hangs now"));
    Serial.flush();
    for (;;) {
    }
  }
#endif
#ifdef TEST_HANG_CLI_AFTER_MS
  // Hang with interrupts disabled: the watchdog interrupt cannot run, so the
  // second timeout must reset the chip (datasheet, WDTCSR bit WDIE).
  if (!(gResetFlags & _BV(WDRF)) && now > TEST_HANG_CLI_AFTER_MS) {
    Serial.println(F("TEST: hang with interrupts off"));
    Serial.flush();
    noInterrupts();
    for (;;) {
    }
  }
#endif
}
