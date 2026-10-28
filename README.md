# Cloudline AI-CLS6 fan controller

A stand-alone speed controller for the **AC Infinity Cloudline AI-CLS6**
(Cloudline S6) EC exhaust fan, using an **Arduino Nano** and an **AITRIP
1.3″ OLED module with an integrated EC11 rotary encoder**. It plugs into the
fan's 4-pin Molex control lead in place of the stock controller and runs
entirely from the fan's own 10 V supply.

| | |
|---|---|
| ![Fan off](docs/img/screen_off.svg) | ![Fan on](docs/img/screen_on.svg) |

## What it does

- **Turn the knob** to set the speed, 0–100 % in 1 % steps (5 % per click
  when you spin it fast).
- **Press the knob** to switch the fan on or off.
- **The fan is always off at power-up.** This is enforced in hardware and in
  firmware.
- **The setting is kept while powered.** Switching off and on returns to the
  same speed. After a power cycle the setting resets to 50 %.
- **The display shows:**
  - ON/OFF status in the title bar, which is inverted while ON;
  - the **setting** as a bar and a percentage;
  - the **measured speed** from the fan's tachometer, as a bar, a percentage
    of the calibrated maximum, and RPM.
- **Extras:**
  - a 3-second hold calibrates "100 %" against the real fan, stored in
    EEPROM;
  - a "NO TACH / STALLED" warning;
  - a diagnostics page for bring-up.
- **Power:** from the fan's +10 V, through a PTC fuse, reverse-polarity
  diode, TVS clamp and decoupling, into the Nano's regulator. A separate
  3.3 V LDO feeds the OLED. The whole controller draws about 45–60 mA from
  the fan.

## Controls

| Input | Action |
|---|---|
| Turn the knob | Change the setting (also works while OFF; the fan stays off) |
| Press the knob | Fan ON / OFF |
| Hold the knob 3 s (fan ON) | Calibrate. The fan runs at 100 % for 18 s (12 s spin-up, 6 s measuring); the result is saved to EEPROM. Press the knob or BAK to cancel. |
| CON key | Diagnostics page (raw tach Hz, pulse count, PWM duty, Timer1 values) |
| BAK key | Back to the main screen / cancel calibration |
| Knob held while powering up | Start on the diagnostics page. That press doesn't toggle the fan. |

The CON and BAK keys are optional. Leave D6/D7 unconnected and everything
else still works.

## ⚠️ Read before connecting

AC Infinity does not publish the electrical details of the control
connector. **Work through [docs/VERIFICATION.md](docs/VERIFICATION.md)
with a multimeter before connecting the controller to your fan.** It takes
about 20 minutes. What comes from builder reports rather than documentation:

- **Pin assignments:** pin 4 = +10 V, pin 3 = GND, pin 2 = speed, pin 1 =
  tach. Identify pins by measuring, not by wire colour.
- **The speed signal is "backwards":** the line held low stops the fan, and
  released (pulled up inside the fan) runs it at full speed. That's why Q1
  pulls the line low instead of driving it. A 3.3 V output driving the line
  directly only reached about half speed in one builder's tests.
- **The tach pin is the least certain.** Nobody has published a measurement
  of it. If your fan has no tach on this connector, the controller still
  works, but the ACT bar shows `--%` with a "NO TACH" warning.

The board is designed so the likely surprises need one change:

| Surprise | Fix |
|---|---|
| Speed input floats (no internal pull-up) | Fit R5 (4.7 kΩ, normally left off) |
| Speed polarity is the other way round | `FAN_PWM_INVERTED 0` in `config.h` **and** move R4 to GND |
| Fan rail measures over 10.8 V | Use a P6KE15A for D2 |
| No tach | Nothing to change; the warning is expected |

For J1, buy a 4-pin Molex extension cable, cut it in half, and keep the end
that plugs into the fan's control lead.

**Failure behaviour:** if the controller loses its 5 V supply while the fan
still has power, the fan runs at **full speed**. I chose that so an exhaust
fan keeps ventilating if the controller dies.
[DESIGN_NOTES.md §12](docs/DESIGN_NOTES.md#12-limitations-and-possible-extensions)
describes how to make it stop instead.

## Documentation

| Document | Contents |
|---|---|
| [docs/schematic.svg](docs/schematic.svg) | Full graphical schematic: every resistor, transistor and connection |
| [docs/PINOUT.md](docs/PINOUT.md) | All pinouts (fan connector, Nano, OLED module, transistors, regulator), power rails and budget, and a point-to-point wiring list |
| [docs/VERIFICATION.md](docs/VERIFICATION.md) | Measuring the fan connector, bench bring-up without the fan, first connection, calibration |
| [docs/DESIGN_NOTES.md](docs/DESIGN_NOTES.md) | Requirements traceability, circuit calculations, firmware architecture, testing, tuning, troubleshooting |
| [docs/BOM.md](docs/BOM.md) | Parts list with part numbers and substitutes |
| [firmware/cloudline_controller/](firmware/cloudline_controller/) | Arduino sketch (`config.h` holds every tunable value) |

[![Schematic](docs/schematic.svg)](docs/schematic.svg)

## Circuit at a glance

| Block | Parts | Job |
|---|---|---|
| Power input | F1 PTC, D1 1N5817, D2 P6KE13A, C1 100 µF, C2 100 nF | Fan +10 V → protected ~9.6 V → Nano **VIN** |
| 3.3 V supply | U2 MCP1700-3302E, C3, C4 | Nano 5 V → 3.3 V for the OLED module |
| Speed output | Q1 2N7000, R3 220 Ω, R4 10 k, (R5 DNP) | D9 PWM pulls the fan's speed line low (open-drain). R4 keeps the fan OFF while the Nano boots. |
| Tach input | Q2 2N3904, R6–R9, C5 | Fan tach (3.3–12 V or open-collector) → clean 5 V pulses on D2 |
| UI | U3 module, R1/R2 4.7 k I²C pull-ups | SH1106 display on A4/A5. Encoder on D3/D4, push on D5, optional keys on D6/D7. |

Choices worth knowing about:

- **Why a separate 3.3 V regulator:** the OLED module is rated for 3.3 V.
  The Nano's own `3V3` pin isn't used because on most CH340 clones it comes
  from the USB chip and isn't meant to power anything.
- **Why R4 exists:** every Nano pin floats while the board boots. R4 holds
  Q1 on during that time, so the fan line stays low and the fan stays off.
  The firmware then takes over without a glitch.
- **R3 is 220 Ω** (not the usual 100 Ω), to keep the D9 pin's peak current
  near 20 mA, under the ATmega's 40 mA limit.
- **R8 is 10 kΩ** (not 47 kΩ): with 47 kΩ, a weak tach "low" of 0.8 V would
  half-turn-on Q2. With 10 kΩ it stays firmly off.

## Building the firmware

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x).
2. **Library Manager:** install **U8g2** by olikraus.
3. Open `firmware/cloudline_controller/cloudline_controller.ino`.
4. **Tools → Board:** Arduino Nano. **Processor:** *ATmega328P*, or
   *ATmega328P (Old Bootloader)* for most clones. If an upload fails, try
   the other one.
5. Review `config.h`, especially after doing VERIFICATION.md part A. Then
   upload.

Command-line alternative:

```sh
arduino-cli lib install U8g2
arduino-cli compile -b arduino:avr:nano firmware/cloudline_controller
arduino-cli upload  -b arduino:avr:nano -p /dev/tty.usbserial-XXXX firmware/cloudline_controller
# clones with the old bootloader: -b arduino:avr:nano:cpu=atmega328old
```

## Host tests

The firmware logic and the whole sketch run on a desktop compiler against
stubs of the Arduino, AVR and U8g2 APIs. The simulation drives the knob,
buttons and a tach pulse train, and checks the PWM registers, displayed
values and the layout of every frame.

```sh
make -C tests          # build and run all suites
make -C tests shots    # regenerate docs/img/screen_*.svg from the real drawing code
python3 tools/gen_schematic.py   # regenerate docs/schematic.svg
```

What the suites cover:

- **Logic tests:** 238 checks, built as C++11 (the standard avr-gcc uses for
  Arduino) and C++17. They cover encoder decoding, button handling, the duty
  mapping, Timer1 maths, the tach speed estimate, smoothing and the EEPROM
  record.
- **Simulation:** five scenarios and about 1,200 checks, running the
  unmodified sketch millisecond by millisecond:
  - `boot1`: the full user journey, including calibration and cancel;
  - `boot2`: after a power cycle;
  - `diagboot`: knob held at power-up;
  - `wrap`: running through the 32-bit `millis()` rollover;
  - `calfail`: calibration with no tach.

  Every rendered frame is checked for text off the screen or overlapping.

The simulation found one real bug during development. The "no tach" check
compared two `millis()` timestamps directly, which would have shown a false
warning after about 49.7 days of uptime. It now compares elapsed times.

See [DESIGN_NOTES.md §9](docs/DESIGN_NOTES.md#9-testing) for details.

## Project status

- **Not yet compiled for the Nano.** The sketch hasn't been built with
  avr-gcc, because the environment it was written in couldn't download the
  Arduino toolchain. The first Arduino IDE compile is the next step. If it
  reports errors, they'll be in the hardware code in the `.ino`, not in the
  tested logic. If it warns about low memory, set `OLED_PAGE_BUFFER 1` in
  `config.h`.
- **Not yet run on hardware**, and the fan's electrical behaviour is
  unconfirmed until you do the measurements in VERIFICATION.md.
- **No watchdog.** The old bootloader on many Nano clones resets in a loop
  after a watchdog reset, so it's left off. See DESIGN_NOTES.md §8 for how
  to enable it on boards with the newer Optiboot bootloader.

## Repository layout

```
├── TASK.md                         original brief
├── README.md
├── docs/
│   ├── schematic.svg               graphical schematic (generated)
│   ├── PINOUT.md  VERIFICATION.md  DESIGN_NOTES.md  BOM.md
│   └── img/screen_*.svg            display screenshots (generated by the tests)
├── firmware/cloudline_controller/
│   ├── cloudline_controller.ino    sketch
│   ├── config.h                    pins and tunables
│   ├── fan_logic.h                 hardware-independent logic (unit-tested)
│   └── display_ui.h                screen layouts
├── tests/                          host unit tests, simulation, Arduino/U8g2 stubs
└── tools/gen_schematic.py          schematic generator (standard library only)
```

## Sources

- [RollItUp: ESP32 control of Cloudline EC fans](https://www.rollitup.org/t/idiots-guide-to-getting-an-esp32-to-control-ac-infinity-cloudline-ec-fans-esphome-ha.1053910/): 4-pin Molex, pin mapping, +10 V, MOSFET low-side drive, inverted signal, 1–5 kHz PWM
- [RollItUp: Cloudline AI-CLS6 PWM control with RPi](https://www.rollitup.org/t/cloudline-ai-cls6-s6-pwm-control-with-rpi.1046753/page-3): experiments on this exact model
- [Home Assistant Community: multiple Cloudline S6 on one ESP32](https://community.home-assistant.io/t/connect-multiple-cloudline-s6-to-single-esp32/592211): half speed with direct GPIO drive, fixed with a MOSFET
- [AC Infinity Cloudline manual (PDF)](https://cdn.shopify.com/s/files/1/1567/1791/files/20210903_Cloudline_Manual.pdf): "4-pin molex connector" between fan and controller
- [makerselectronics](https://makerselectronics.com/product/oled-display-i2c-1-3-inch-module-with-ec11-rotary-encoder/) and [bitsandparts](https://www.bitsandparts.nl/en/display-oled-128x160-1.3-inch-i2c-white-with-ec11-rotary-encoder-p1931809) listings for the 1.3″ OLED + EC11 module: SH1106, I²C 0x3C, 3.3 V, pin labels, 20 pulses / 20 detents
