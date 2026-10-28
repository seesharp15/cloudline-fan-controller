# Tests

| Suite | What runs | Command |
|---|---|---|
| Logic unit tests (`test_fan_logic.cpp`) | `fan_logic.h` on the host, built as C++11 and C++17 | `make -C test` |
| Firmware simulation (`test_firmware_sim.cpp`) | the unmodified `app.cpp` against Arduino/AVR/U8g2 stubs in `stubs/`, millisecond by millisecond | `make -C test` |
| No-EEPROM check | greps the firmware for any EEPROM access | `make -C test` |
| Pin cross-check (`check_pins.py`) | parses `config.h`, the schematic and wiring sources, PINOUT.md and README.md and fails if any pin assignment disagrees | `make -C test` (needs python3) |
| AVR simulation (`avr/`) | the real Nano binary in simavr, including a watchdog recovery | `sh test/avr/run_simavr.sh` |

`make -C test` needs only a C++17 compiler (g++ or clang++) and make. It
prints one line per test case and a summary per suite, and ends with
`ALL TESTS PASSED`. `make -C test shots` regenerates the screen images in
`docs/img/` from the real drawing code.

Simulation scenarios (one process per power-up):

| Scenario | Covers |
|---|---|
| `boot1` | power-up state, encoder (slow/fast/clamp/bounce), ON/OFF, tach RPM and timeout, NO TACH warning, calibration by long press, cancel, button bounce, diagnostics page, 10 Hz refresh, watchdog feeding |
| `powercycle` | a new power-up keeps nothing (OFF, default setting, max RPM unknown) |
| `serial` | every console command, invalid input, over-long lines |
| `diagboot` | knob held at power-up opens diagnostics without toggling the fan |
| `wrap` | running across the 32-bit `millis()` rollover (49.7 days) |
| `calfail` | calibration with no tach signal |

Each frame the firmware draws is checked for text outside the 128x64 screen
and for overlapping text.

What these tests cannot cover: anything electrical (the fan, Q1, the tach
front end, the power supply, the real OLED). That is what
[BENCH_TEST.md](../BENCH_TEST.md) is for.
