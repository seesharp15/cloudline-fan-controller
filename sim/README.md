# Wokwi simulation

Try the user interface (screens, knob, buttons, serial console) in the
[Wokwi](https://wokwi.com) browser simulator before buying any hardware.

> **Status: prepared but not run by the author.** The environment this
> project was built in could not reach wokwi.com. The firmware's Wokwi build
> compiles (26 746 bytes flash, 1 293 bytes RAM), and `diagram.json` uses the
> part and pin names from Wokwi's documentation, but nobody has watched it run
> yet. If a connection doesn't work, check the pin names in Wokwi's part
> reference. The same UI logic *is* covered by the host simulation in
> `test/` (screenshots in `docs/img/`).

## Files

| File | Purpose |
|---|---|
| `diagram.json` | Nano, SSD1306 OLED, KY-040 encoder (stands in for the EC11 and its push switch), CON/BAK buttons, an LED on D9 and a logic analyzer on D9/D2, and a wire from D8 to D2 for the fake tach |
| `libraries.txt` | Library list for wokwi.com (`U8g2`) |
| `wokwi.toml` | For the Wokwi VS Code extension or `wokwi-cli`: points at a locally built firmware |

## Run it on wokwi.com

1. Create a new **Arduino Nano** project.
2. Replace `diagram.json` with the one here. Add `libraries.txt` (or add U8g2
   in the *Library Manager* tab).
3. Add the firmware files as tabs: `config.h`, `app.cpp`, `fan_logic.h`,
   `display_ui.h`, and paste `cloudline_controller.ino` into `sketch.ino`.
4. In `config.h` set `#define TARGET_WOKWI 1` (on the line inside the
   `#ifndef TARGET_WOKWI` block).
5. Start the simulation.

## Run it locally (VS Code extension or wokwi-cli)

```sh
arduino-cli compile -b arduino:avr:nano \
  --build-property "compiler.cpp.extra_flags=-DTARGET_WOKWI=1" \
  --output-dir sim/build firmware/cloudline_controller
# then open sim/ with the Wokwi extension, or: wokwi-cli sim
```

## What `TARGET_WOKWI 1` changes

- The display driver is **SSD1306** (Wokwi has no SH1106).
- A **fake tach** is generated on D8 (wired to D2 in the diagram): a square
  wave at the frequency a 2000 RPM fan would produce at the current line
  duty (or at `TACH_MAX_RPM` if you set one). `tone()` can't go below 31 Hz,
  so settings below about 47 % read as stopped and show `NO TACH` after 8 s.

## What can and cannot be simulated

| Can be exercised | Cannot be exercised |
|---|---|
| All screens and the layout; the encoder, push switch, CON/BAK keys | The SH1106 itself (Wokwi has only the SSD1306) |
| ON/OFF state machine, off at power-up, setting kept while running, reset back to defaults | The fan: its pull-up, polarity, minimum duty, whether 0 % stops it, its real tach signal and pulses per revolution |
| D9 PWM: the LED is lit when the **gate** is high, i.e. it is brightest when the fan would be **slowest** (inverted drive); the logic analyzer shows the 4 kHz waveform | Q1, R4 and the boot-time hold-off (no MOSFET or analogue pull-up behaviour) |
| RPM display, NO TACH, calibration (with the fake tach) | The tach front end (R6/R7/D3/D4/C5) and noise |
| Serial console (Wokwi's serial monitor) | Power supply, regulator heat, brown-out, reverse polarity, the fan's current limit |
| Watchdog: not checked in Wokwi (the AVR simulation in `test/avr` covers it) | Anything in the pre-connection verification |
