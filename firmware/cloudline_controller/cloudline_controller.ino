// Cloudline AI-CLS6 fan controller
// Arduino Nano (ATmega328P, 16 MHz) + AITRIP 1.3" SH1106 OLED with EC11 encoder.
//
//   Turn the knob ........ change the speed setting (0-100 %, also while OFF)
//   Press the knob ....... fan ON / OFF (always OFF after power-up)
//   Hold the knob 3 s .... (fan ON) calibrate: learn the RPM at 100 %
//   CON key .............. diagnostics page on / off
//   BAK key .............. back / cancel calibration
//   Knob held at power-up  start on the diagnostics page
//   Serial 115200 baud ... send ? for commands
//
// The speed setting lives in RAM only: it survives ON/OFF toggles and is
// reset to SETTING_DEFAULT_PCT when power is removed. Nothing is saved to
// non-volatile memory (the test suite checks this).
//
// Files in this sketch:
//   config.h      every tunable value and pin number: EDIT THIS ONE
//   app.cpp       hardware glue: setup(), loop(), interrupts, PWM, watchdog
//   fan_logic.h   pure logic (encoder, buttons, duty, tach, state machine)
//   display_ui.h  screen layouts
//
// This file intentionally contains no code. Keeping the functions in .cpp
// files means the Arduino IDE does not generate prototypes for them, so the
// IDE build and the host unit tests compile exactly the same source.
//
// Required library: U8g2 by olikraus (see ../libraries.txt for the version).

#include "config.h"
