// config.h - every tunable value for the Cloudline AI-CLS6 controller.
//
// =========================================================================
//  CONFIGURATION BLOCK - this is the only file you should need to edit.
// =========================================================================
// It is included first by the sketch. See DESIGN_NOTES.md in docs/ for the
// reasoning behind each default and README.md "Pre-connection verification"
// for how to measure the values on your own fan.
#pragma once

// ---------------------------------------------------------------------------
// Build target
// ---------------------------------------------------------------------------
// 0 = real hardware (Arduino Nano + AITRIP SH1106 module).
// 1 = Wokwi simulator: SSD1306 display, and a fake tach signal generated on
//     PIN_SIM_TACH_OUT (wire it to D2 in the simulator). See sim/README.md.
#define TARGET_WOKWI 0

// ---------------------------------------------------------------------------
// Pin assignments (Arduino Nano, ATmega328P). Must match PINOUT.md.
// ---------------------------------------------------------------------------
// D2 is INT0, the only pin with a dedicated interrupt that is left after D3
// is used by the encoder: tach pulses are timed here.
#define PIN_TACH        2
// D3/D4 are both on port D, so one pin-change interrupt (PCINT2) catches
// every edge of the EC11 quadrature signals.
#define PIN_ENC_A       3   // module pin TRA
#define PIN_ENC_B       4   // module pin TRB
#define PIN_ENC_SW      5   // module pin PSH (encoder push switch), polled
#define PIN_KEY_CON     6   // module pin CON (optional: diagnostics page), polled
#define PIN_KEY_BAK     7   // module pin BAK (optional: back / cancel), polled
#define PIN_SIM_TACH_OUT 8  // Wokwi only: fake tach output (unused on hardware)
// D9 is OC1A: Timer1 hardware PWM drives the Q1 gate.
#define PIN_FAN_PWM     9
#define PIN_STATUS_LED  13  // on-board LED, lit while the fan is commanded on
// I2C uses the fixed hardware pins A4 (SDA) and A5 (SCL).

// ---------------------------------------------------------------------------
// Fan PWM output
// ---------------------------------------------------------------------------
// Polarity. Q1 is an open-drain pull-down, so with the fan's own pull-up the
// gate drive is inverted: gate HIGH = line LOW = fan slower/stopped.
// 1 = line LOW stops the fan (builder reports for Cloudline fans).
// 0 = only if your measurements show the opposite. Then also move R4 so it
//     pulls the gate to GND, otherwise the fan runs at boot.
#define FAN_PWM_INVERTED   1

// PWM carrier frequency on the fan's speed line, in Hz. The stock AC Infinity
// controller was measured at about 4 kHz (245 us period); builders report
// 1 kHz and ~5 kHz also working. Valid range: 250..20000. Timer1 runs without
// a prescaler, so resolution is 16 MHz / FAN_PWM_FREQ_HZ steps (4000 at 4 kHz).
#define FAN_PWM_FREQ_HZ    4000UL

// Line duty (%) for a 1 % and a 100 % setting. Settings 1..100 map linearly
// onto (FAN_DUTY_MIN_PCT, FAN_DUTY_MAX_PCT]. If the fan does not turn at the
// bottom of the knob, raise FAN_DUTY_MIN_PCT to the lowest duty that turns it
// (BENCH_TEST.md step R6). OFF and a 0 % setting always give 0 % duty.
#define FAN_DUTY_MIN_PCT   0
#define FAN_DUTY_MAX_PCT   100

// ---------------------------------------------------------------------------
// Speed setting and encoder
// ---------------------------------------------------------------------------
#define SETTING_DEFAULT_PCT   50  // setting at power-up (the fan still starts OFF)
#define SETTING_MIN_PCT       0
#define SETTING_MAX_PCT       100
#define SETTING_STEP_PCT      1   // per encoder click when turned slowly
#define SETTING_FAST_STEP_PCT 5   // per click when clicks are < ENC_FAST_MS apart
#define ENC_FAST_MS           40
// 4 = one full quadrature cycle per click (EC11 20 pulses / 20 detents, the
//     usual part on these modules). 2 = half a cycle per click (15 / 30).
//     Symptom of the wrong value: every other click is ignored (use 2), or
//     one click moves 2 steps (use 4).
#define ENC_EDGES_PER_DETENT  4
// Set to 1 if turning clockwise lowers the speed on your module.
#define ENC_REVERSE           0

// ---------------------------------------------------------------------------
// Tachometer
// ---------------------------------------------------------------------------
// Tach pulses per motor revolution. Only affects the RPM number; the ACT %
// is relative to TACH_MAX_RPM measured with the same setting.
#define TACH_PULSES_PER_REV     2
// "100 %" speed for the ACT bar, in RPM as computed with TACH_PULSES_PER_REV.
// AC Infinity publishes no RPM figure for the AI-CLS6, so the default is 0 =
// unknown: the ACT percentage shows "--" until you calibrate (hold the knob
// 3 s while the fan is ON, or send "c"). A calibration lasts until power-off;
// to keep it, copy the number the controller prints on the serial console
// into here.
#define TACH_MAX_RPM            0
// Pulses closer together than this are treated as noise and ignored.
// 1000 us allows up to 30 000 RPM at 2 pulses/rev.
#define TACH_MIN_PERIOD_US      1000UL
// With no pulse for this long the measured speed reads 0 RPM.
#define TACH_TIMEOUT_MS         1500UL
// Fan commanded on but no tach pulse for this long -> "NO TACH" warning.
// EC fans ramp up slowly, so allow several seconds after switching on.
#define TACH_FAULT_MS           8000UL
// RPM measurement window. Longer = steadier reading, slower response.
#define TACH_WINDOW_MS          500UL
// Exponential smoothing of the displayed RPM, 1..16 (higher = smoother).
#define TACH_SMOOTHING          4

// ---------------------------------------------------------------------------
// Calibration (hold the knob CAL_HOLD_MS while the fan is ON, or send "c")
// ---------------------------------------------------------------------------
#define CAL_HOLD_MS             3000UL
#define CAL_SPINUP_MS           12000UL  // run at 100 % this long before sampling
#define CAL_SAMPLE_MS           6000UL   // then average the RPM over this long
#define CAL_MIN_VALID_RPM       200      // reject calibrations below this
#define CAL_RESULT_SHOW_MS      4000UL

// ---------------------------------------------------------------------------
// Buttons, display, serial
// ---------------------------------------------------------------------------
#define BUTTON_DEBOUNCE_MS      25
#define DISPLAY_REFRESH_MS      100     // periodic redraw: 10 Hz
#define DISPLAY_MIN_GAP_MS      30      // fastest redraw after a knob turn
#define OLED_I2C_ADDRESS        0x3C    // 7-bit address; 0x3D on some boards
#define OLED_I2C_CLOCK_HZ       400000UL
// The AITRIP 1.3" module is sold as SH1106. If the picture is shifted by two
// pixels with a column of garbage at one edge, your board has an SSD1306:
// set this to 1.
#define OLED_SSD1306            0
// 1 = 256-byte page buffer (default). 0 = full 1 KB frame buffer, which does
// NOT fit: with Serial, Wire and U8g2 the full buffer leaves ~4 bytes of the
// Nano's 2 KB RAM for the stack. Neither mode flickers: U8g2 writes the
// display RAM page by page and never clears the panel between frames.
#define OLED_PAGE_BUFFER        1
#define SERIAL_BAUD             115200UL

// ---------------------------------------------------------------------------
// Watchdog
// ---------------------------------------------------------------------------
// If loop() stops running for WATCHDOG_TIMEOUT, the watchdog interrupt
// stops the fan (Q1 on) and restarts the sketch from address 0 without a
// hardware reset, so it also works on Nanos with the old bootloader (which
// reset-loops after a hardware watchdog reset). If even the interrupt cannot
// run, a second timeout performs a hardware reset; R4 then holds the fan off.
// Either way the sketch starts again with the fan OFF, like a power-up.
// Allowed timeouts: WDTO_250MS, WDTO_500MS, WDTO_1S, WDTO_2S, WDTO_4S, WDTO_8S.
#define WATCHDOG_ENABLE         1
#define WATCHDOG_TIMEOUT        WDTO_2S

// ---------------------------------------------------------------------------
// Derived settings (do not edit)
// ---------------------------------------------------------------------------
#if TARGET_WOKWI
#undef OLED_SSD1306
#define OLED_SSD1306 1   // Wokwi simulates an SSD1306, not an SH1106
#define SIM_FAKE_TACH 1
#else
#define SIM_FAKE_TACH 0
#endif
