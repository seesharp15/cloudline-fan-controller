// config.h - build-time settings for the Cloudline AI-CLS6 controller.
//
// Everything you might need to tune for your particular fan lives here.
// See docs/DESIGN_NOTES.md for the reasoning behind each default and
// docs/VERIFICATION.md for how to measure the values on your own fan.
#pragma once

// ---------------------------------------------------------------------------
// Pin assignments (Arduino Nano, ATmega328P)
// ---------------------------------------------------------------------------
// D2 is INT0: tach pulses arrive here from the Q2 buffer transistor.
#define PIN_TACH        2
// D3/D4 sit on the PCINT2 port and decode the EC11 quadrature signals.
#define PIN_ENC_A       3   // module pin TRA
#define PIN_ENC_B       4   // module pin TRB
#define PIN_ENC_SW      5   // module pin PSH (encoder push switch)
#define PIN_KEY_CON     6   // module pin CON / KEY1 (optional: diagnostics page)
#define PIN_KEY_BAK     7   // module pin BAK / KEY0 (optional: back / cancel)
// D9 is OC1A: Timer1 hardware PWM drives the Q1 gate.
#define PIN_FAN_PWM     9
#define PIN_STATUS_LED  LED_BUILTIN  // D13, lit while the fan is commanded on
// I2C uses the fixed hardware pins A4 (SDA) and A5 (SCL).

// ---------------------------------------------------------------------------
// Fan PWM output
// ---------------------------------------------------------------------------
// The Cloudline speed input is pulled up inside the fan. Q1 is an open-drain
// pull-down, so the gate drive is inverted: gate HIGH = line LOW = slower.
// Field reports: line held LOW = fan stopped, line released (HIGH) = full speed.
// Set to 0 only if docs/VERIFICATION.md step 5 shows the opposite on your fan.
#define FAN_PWM_INVERTED   1

// PWM carrier frequency. 1 kHz and 5 kHz are both reported working on
// Cloudline EC fans. Valid range here: 250 Hz .. 20 kHz.
#define FAN_PWM_FREQ_HZ    1000UL

// Mapping from the user's speed setting (1..100 %) to the duty cycle on the
// fan's speed line. Some EC fans will not start below ~20 % duty; raise
// FAN_DUTY_MIN_PCT so that "1 %" is the slowest speed that still turns.
// 0 % setting with the fan ON still produces 0 % duty (stopped).
#define FAN_DUTY_MIN_PCT   0
#define FAN_DUTY_MAX_PCT   100

// ---------------------------------------------------------------------------
// Speed setting
// ---------------------------------------------------------------------------
#define SETTING_DEFAULT_PCT  50   // setting loaded at power-up (fan stays OFF)
#define SETTING_MIN_PCT      0
#define SETTING_MAX_PCT      100
#define SETTING_STEP_PCT     1    // per encoder detent when turned slowly
#define SETTING_FAST_STEP_PCT 5   // per detent when turned quickly
#define ENC_FAST_MS          40   // detents closer than this count as "fast"
// Set to 1 if turning clockwise lowers the speed on your module.
#define ENC_REVERSE          0

// ---------------------------------------------------------------------------
// Tachometer
// ---------------------------------------------------------------------------
// Tach pulses per motor revolution. Only affects the RPM number; the
// "actual %" bar is relative to the calibrated maximum and does not depend
// on this. Use the diagnostics page (CON key or hold the knob at power-up)
// to read the raw frequency and compare it with a known speed.
#define TACH_PULSES_PER_REV     2
// Assumed maximum speed until a calibration has been run and saved.
#define TACH_DEFAULT_MAX_RPM    2000
// Pulses closer together than this are treated as noise (debounce).
// 1000 us allows up to 30 000 RPM at 2 pulses/rev.
#define TACH_MIN_PERIOD_US      1000UL
// With no pulse for this long the fan is reported as stopped.
#define TACH_TIMEOUT_MS         1500UL
// Fan commanded on but no tach pulse for this long -> "NO TACH" warning.
// EC fans ramp up slowly, so allow several seconds after switching on.
#define TACH_FAULT_MS           8000UL
// RPM measurement window. Longer = steadier reading, slower response.
#define TACH_WINDOW_MS          500UL
// Exponential smoothing of the displayed RPM, 1..16 (higher = smoother).
#define TACH_SMOOTHING          4

// ---------------------------------------------------------------------------
// Calibration (hold the knob for CAL_HOLD_MS while the fan is ON)
// ---------------------------------------------------------------------------
#define CAL_HOLD_MS             3000UL
#define CAL_SPINUP_MS           12000UL  // run at 100 % this long before sampling
#define CAL_SAMPLE_MS           6000UL   // then average RPM over this long
#define CAL_MIN_VALID_RPM       200      // reject calibrations below this

// ---------------------------------------------------------------------------
// Buttons and display
// ---------------------------------------------------------------------------
#define BUTTON_DEBOUNCE_MS      25
#define DISPLAY_REFRESH_MS      100     // periodic redraw (10 Hz)
#define DISPLAY_MIN_GAP_MS      30      // fastest redraw after a knob turn
#define OLED_I2C_ADDRESS        0x3C    // 7-bit; module default
#define OLED_I2C_CLOCK_HZ       400000UL
// The AITRIP 1.3" module uses an SH1106. If the picture is shifted by two
// pixels with a garbage column at one edge, your board has an SSD1306:
// set this to 1.
#define OLED_SSD1306            0
// 0 = full frame buffer (1 KB RAM, fastest). 1 = 256-byte page buffer, for
// when you add features and run short of the Nano's 2 KB of RAM.
#define OLED_PAGE_BUFFER        0
