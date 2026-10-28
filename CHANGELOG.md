# Changelog

## Rev B (2026-10-28): addendum - documentation, verification and tests

Hardware
- Tach input redesigned: pulled up to **+5 V** (R6 10 kΩ) behind a 1 kΩ
  series resistor (R7) with BAT85 clamps to +5V and GND (D3, D4) and a
  10 nF filter (C5). Replaces the 2N3904 inverter that pulled the fan's tach
  pin up to 10 V. D2 now never sees more than ~5.3 V, whatever is on J1 pin 1.
- C1 raised from 100 µF to 220 µF for more hold-up during supply dips.
- Schematic regenerated (SVG + PNG) with the J1 mating-face view; new
  wiring view with wire colours (`hardware/wiring.svg`), overview
  illustration, BOM as CSV with part numbers and prices.

Firmware
- First real AVR build (arduino-cli, avr-gcc 7.3.0). It showed that the full
  1 KB frame buffer leaves only 4 bytes of RAM: switched to the 256-byte
  page buffer, moved all UI strings to flash, and use the smaller `_tr`
  fonts. Now 25 404 bytes flash (82 %), 1 276 bytes RAM (62 %).
- **No EEPROM at all.** The calibration result is kept in RAM until
  power-off and printed on the serial console for copying into `config.h`.
  The max RPM defaults to "unknown" (`--%`) because no RPM figure is
  published for the AI-CLS6.
- **Watchdog** added (2 s), using interrupt-then-reset mode: the interrupt
  stops the fan and restarts the sketch without a hardware reset, so it is
  safe on old-bootloader Nanos.
- PWM default changed from 1 kHz to **4 kHz**, matching a builder's
  measurement of the stock controller.
- Serial console (115200 baud): status, on/off, set, calibrate, max RPM.
- Logic restructured: the ON/OFF/setting/calibration state machine and the
  command parser moved into `fan_logic.h` (`fan::Controller`) and are unit
  tested; the hardware glue moved from the `.ino` into `app.cpp`.
- Encoder decoder supports 2 or 4 edges per detent (`ENC_EDGES_PER_DETENT`).
- Compile-time checks on pin and range settings.

Tests
- `tests/` renamed to `test/`. New unit tests for the controller state
  machine, command parser, half-step encoders, rapid reversals, button
  bounce, duty and polarity edge cases, tach fault and the divide-by-zero
  guard (1 107 checks). New simulation scenarios (`powercycle`, `serial`).
- `test/avr/run_simavr.sh` runs the real Nano binary in simavr and checks
  boot and watchdog recovery.
- Wokwi project in `sim/` (prepared, not run).

Documentation
- New README (complete build guide), FAQ, BENCH_TEST, VERIFICATION (report),
  root PINOUT. `docs/DESIGN_NOTES.md`, `docs/VERIFICATION.md` (old procedure)
  and `docs/BOM.md` folded into these.

## Rev A (2026-10-28)

- Initial design: schematic, pinouts, firmware, host tests.
