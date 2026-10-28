# Verification report

What has been checked, how, and what still has to be confirmed on hardware.
Date of this report: 2026-10-28, firmware and hardware **Rev B**.

**Bottom line.** The firmware compiles for the Nano with adequate memory
headroom, its logic passes all host tests, and the real compiled binary boots
and recovers from a hang in an AVR simulator. The circuit values are
calculated below from datasheet limits. **Nothing has been built or run on a
real Nano, OLED module or fan.** The fan-side facts (pinout, signal levels,
tach) come from builder reports, not from AC Infinity, and must be confirmed
with the README's [pre-connection verification](README.md#pre-connection-verification)
and [BENCH_TEST.md](BENCH_TEST.md). This report does not claim the design
will work beyond what that evidence supports.

## Contents

1. [Requirements traceability matrix](#1-requirements-traceability-matrix)
2. [Electrical calculations](#2-electrical-calculations)
3. [Pin assignment cross-check](#3-pin-assignment-cross-check)
4. [Verified versus assumed](#4-verified-versus-assumed)
5. [Hardware assumptions: evidence and status](#5-hardware-assumptions-evidence-and-status)
6. [Known limitations and failure modes](#6-known-limitations-and-failure-modes)
7. [Independent review: findings and fixes](#7-independent-review-findings-and-fixes)
8. [Recorded test and build output](#8-recorded-test-and-build-output)

Status legend:

- **PASS**: met and verified by the evidence named (test, compile,
  calculation, inspection).
- **PASS (sim)**: met in software and simulation; hardware confirmation is
  still listed in the "bench" column.
- **HW PENDING**: depends on a fact or behaviour only a physical test can
  confirm.
- **FLAGGED**: cannot be verified from the sources available; documented with
  a procedure for the builder.

---

## 1. Requirements traceability matrix

Code locations are in `firmware/cloudline_controller/` unless a path is
given. "Unit" = `test/test_fan_logic.cpp`, "Sim" = `test/test_firmware_sim.cpp`
scenario, "AVR" = `test/avr/run_simavr.sh`, bench IDs = [BENCH_TEST.md](BENCH_TEST.md).

### 1.1 Original brief (TASK.md)

| ID | Requirement | Code | Schematic | Tests / bench | Status |
|---|---|---|---|---|---|
| R1 | Encoder controls fan speed | `fan_logic.h` `QuadratureDecoder`, `applyDetents`, `Controller::onDetents`; `app.cpp` `ISR(PCINT2_vect)`, `handleEncoder` | U3 TRA/TRB → D3/D4 | Unit `decoder_*`, `detents_*`; Sim `boot1`; bench A6, B4 | PASS (sim) |
| R2 | Encoder button toggles on/off | `fan_logic.h` `Button`, `Controller::onKnob`; `app.cpp` `handleButtons` | U3 PSH → D5 | Unit `button_*`, `controller_on_off_keeps_setting`; Sim `boot1` (incl. bounce); bench A8, B3 | PASS (sim) |
| R3 | Fan off at power-up | `Controller` (`on_ = false`); `app.cpp` `setupFanPwm()` (gate HIGH before OUTPUT), `setup()` | **R4** 10 kΩ gate pull-up, Q1 | Unit `controller_power_up_state`; Sim `checkPowerUpState` in every scenario; AVR `normal` (duty 0.0 %); bench A4, A5, B1, B2 | PASS (sim); HW PENDING (fan polarity, M4) |
| R4 | Setting persists while powered | `Controller::setting_` (RAM); OFF does not change it | — | Unit `controller_on_off_keeps_setting`; Sim `boot1`, `powercycle`; bench A9, B11 | PASS (sim) |
| R5 | Display: setting and tach speed; on/off status, bars, percentages | `display_ui.h` `drawMain`; `app.cpp` `updateDisplay` | U3 SDA/SCL | Sim `boot1` text and layout checks on every frame; screenshots `docs/img/`; bench A10, A14, B9 | PASS (sim); tach HW PENDING (H6) |
| R6 | Arduino and OLED powered from the fan's 10 V, with conditioning | — | F1, D1, D2, C1, C2 → VIN; U2, C3, C4 → +3V3 | Calculations §2.1–2.5; bench A1–A3, A13, B1, B10, B13 | PASS (calc); HW PENDING (H2) |
| R7 | Pinout document incl. power supplies | [PINOUT.md](PINOUT.md) §1–§6 | — | `test/check_pins.py` (§3) | PASS |
| R8 | Graphical schematic incl. resistors and transistors | `hardware/schematic.py` → `.svg`/`.png` | all | Inspection (rendered PNG) | PASS |
| R9 | Arduino code | `firmware/cloudline_controller/` | — | Compile §8.4; all tests | PASS |

### 1.2 Addendum §1: README

| ID | Item | Where | Status |
|---|---|---|---|
| A1.1 | What it does + photo-style overview | README top, `hardware/overview.png` | PASS |
| A1.2 | BOM: part, qty, value/rating, part number, cost; connectors, wire gauge, heat shrink, enclosure | README *Bill of materials*, `hardware/bom.csv` | PASS |
| A1.3 | Tools incl. multimeter, USB cable type | README *Tools* | PASS |
| A1.4 | IDE 2.x, board, pinned library versions, CH340 note, processor setting and how to tell | README *Software setup*, `firmware/libraries.txt` | PASS |
| A1.5 | Build guide in the required order, each step with a check | README steps 1 verify connector → 2 power conditioning → 3 power test without Nano → 4 OLED/encoder → 5 flash and test on USB → 6 fan interface → 7 first power-up on the fan → 8 enclosure; each ends with "Check before continuing" | PASS |
| A1.6 | First power-up and expected display | README *Step 7. First power-up on the fan supply* | PASS |
| A1.7 | Normal operation | README *Normal operation* | PASS |
| A1.8 | Troubleshooting table with all listed symptoms | README *Troubleshooting* (blank, garbled, encoder skip/backwards, button, full speed when off, won't stop at 0 %, RPM 0/erratic, resets on speed change, upload fails) | PASS |

### 1.3 Addendum §2: FAQ

| ID | Question | Status |
|---|---|---|
| A2.1–A2.10 | USB + fan; off at power-up and bootloader window; persistence and how to add it; RPM % and calibration; PWM frequency; other fans; unplugged while running; warranty and damage; Uno/Pro Mini/ESP32; SSD1306 vs SH1106 | PASS: all ten answered in [FAQ.md](FAQ.md), plus six more |

### 1.4 Addendum §3: hardware assumptions

| ID | Assumption | Evidence | Builder check | Status |
|---|---|---|---|---|
| H1 | J1 pinout | §5 H1 | M1, M3, M4, M5 | FLAGGED (builder reports) |
| H2 | Supply voltage and current | §5 H2 | M1, M2 | FLAGGED (voltage reported; current undocumented) |
| H3 | Control signal type, level, frequency | §5 H3 | M3, M4 | FLAGGED (consistent builder reports) |
| H4 | Internal pull-up; floating behaviour | §5 H4 | M3, M3b, M4 | FLAGGED |
| H5 | 0 % stops the fan | §5 H5 | M4, bench B7 | FLAGGED |
| H6 | Tach type, level, PPR | §5 H6 | M5, bench B5 | FLAGGED (**no source found**) |
| H7 | OLED driver, interface, address | §5 H7 | build step 5 | FLAGGED (seller listings) |
| H8 | README pre-connection section with stop rules | README *Pre-connection verification* | — | PASS |

### 1.5 Addendum §4: design issues

| ID | Issue | Code | Schematic | Tests / bench | Status |
|---|---|---|---|---|---|
| D1 | Interrupt allocation | `app.cpp` header comment, `onTachPulse` (INT0), `ISR(PCINT2_vect)`; compile-time `#error` checks | D2 tach, D3/D4 encoder | Sim checks INT0 attached, INT1 unused, PCMSK2 = D3/D4; PINOUT §3 | PASS |
| D2 | Boot-time fan state, hardware hold-off | `setupFanPwm()` | **R4** 10 kΩ Q1 gate → +5V | Sim `outWhenMadeOutput == HIGH`; calc §2.6; bench A4, A5 | PASS (sim/calc); HW PENDING |
| D3 | PWM level shifting into 10 V input; inversion in code | `fan::gateDriveFor`, `FAN_PWM_INVERTED` | **Q1** 2N7000, **R3** 220 Ω, R4, R5 (DNP) | Unit `gate_drive_*`, `polarity_inversion`; Sim duty checks; bench A7 | PASS (sim/calc); HW PENDING (M4) |
| D4 | Tach pulled up to 5 V, protected | `onTachPulse` (FALLING) | **R6** 10 kΩ → +5V, **R7** 1 kΩ, **D3/D4** BAT85, **C5** 10 nF | Calc §2.7; bench A10, A11 | PASS (calc); HW PENDING |
| D5 | Power: VIN regulator vs buck, dissipation at 10 V, reverse polarity, bulk/decoupling, transients, brown-out | — | F1, D1, D2, C1, C2 | Calc §2.2–2.5; FAQ; bench A1–A3, A13, B10 | PASS (calc); HW PENDING |
| D6 | Encoder: state machine, no skip/reverse, step and range | `QuadratureDecoder`, `applyDetents`, `ENC_*`, `SETTING_*` in `config.h` | — | Unit `decoder_*` (bounce, rapid reversal, half-step), `detents_at_limits`; Sim bounce | PASS (sim) |
| D7 | Button: debounced, one toggle per press | `fan::Button` (25 ms) | — | Unit `button_bounce_gives_one_press`; Sim bounce block in `boot1`; bench A8 | PASS (sim) |
| D8 | RPM: window, timeout, % vs calibrated max, calibration method | `TachEstimator`, `smoothRpm`, `rpmToPercent`, `tachFault`, `Controller` calibration; serial `c`/`m` | — | Unit `tach_*`, `rpm_percent*`, `controller_calibration_*`; Sim `boot1`, `calfail`, `serial`; bench A10–A12, B8 | PASS (sim) |
| D9 | Display: no flicker, stated rate, layout | `updateDisplay` (10 Hz + on input), page buffer | — | Sim: 95–105 frames per 10 s, layout checks; bench A14 | PASS (sim); flicker HW PENDING |
| D10 | Setting in RAM only; no EEPROM writes | no EEPROM code anywhere | — | `make -C test` `no-eeprom`; `avr-nm` shows no EEPROM routines linked (§8.4); Sim `powercycle` | PASS |
| D11 | Watchdog; documented reset behaviour | `watchdogStart`, `ISR(WDT_vect)`, `earlyResetHandling` (`.init3`) | R4 keeps fan off during any reset | AVR `hang` (recovery, fan off, `reset=0x80`); Sim feed count; FAQ; bench W1, W2 | PASS (AVR sim); interrupts-off path per datasheet, HW PENDING (W2) |
| D12 | Every tunable in a labelled config block | `config.h` (included first) | — | Inspection | PASS |

### 1.6 Addendum §5–§9

| ID | Item | Evidence | Status |
|---|---|---|---|
| T1 | Pure logic separated from hardware | `fan_logic.h` has no Arduino calls; `app.cpp` glue only | PASS |
| T2 | Host unit tests incl. listed edge cases | `test/test_fan_logic.cpp`: encoder at min/max (`detents_at_limits`), rapid direction changes, button bounce, tach timeout, divide-by-zero, max RPM not calibrated | PASS |
| T3 | Tests run, output included | §8.1–8.3 | PASS |
| T4 | arduino-cli compile with flash/RAM; RAM headroom | §8.4; §2.11 | PASS |
| T5 | Wokwi simulation, can/can't | `sim/`; [sim/README.md](sim/README.md). **Not run**: wokwi.com unreachable from the build environment; stated in sim/README. | PARTIAL (prepared, compiles, not executed) |
| T6 | BENCH_TEST.md, dummy load then real fan | [BENCH_TEST.md](BENCH_TEST.md) Parts A and B | PASS |
| T7 | State what is not possible | sim/README (Wokwi not run), §4, test/avr/README (simavr watchdog limit) | PASS |
| V1–V6 | This report: matrix, calculations, pin cross-check, verified vs assumed, limitations, review findings | §1–§7 | PASS |
| S1 | Schematic SVG + PNG from editable source | `hardware/schematic.py` → `schematic.svg`, `schematic.png` | PASS |
| S2 | Ref + value on every part; connector pins from mating side, side stated | schematic; J1 face view box | PASS |
| S3 | Wiring view with wire colours | `hardware/wiring.svg/.png` | PASS |
| L1 | Repository layout | README *Development and tests* | PASS |
| C1 | Final checklist | end of the delivery message | PASS |

---

## 2. Electrical calculations

Datasheet values are the manufacturers' limits for the parts in the BOM.
"Typ" means a typical figure; worst cases use the limits.

### 2.1 Current budget (+10 V input)

| Load | Current | Basis |
|---|---|---|
| ATmega328P, 16 MHz, 5 V, active | ≈ 10 mA | Datasheet active-supply-current curve (~9–12 mA) |
| CH340 USB chip (idle, no USB) | 3–15 mA | Varies by clone; estimate |
| Nano power LED | ≈ 3 mA | 5 V through ~1 kΩ |
| D13 LED (fan ON) | ≈ 3 mA | 5 V through ~1 kΩ |
| OLED module | 5–20 mA | Depends on lit pixels (these screens light ~20–35 %); estimate |
| R4 (D9 low, fan running) | 0.5 mA | 5 V / 10 kΩ |
| R6 (tach low) | ≤ 0.5 mA | 5 V / 11 kΩ |
| R1/R2 (I²C low) | ≤ 1.5 mA each, while bits are low | 3.3 V / 2.2 kΩ |
| U2 quiescent | 1.6 µA | MCP1700 datasheet |
| **Total** | **≈ 25–55 mA; design figure 60 mA** | |

### 2.2 Input chain

- **F1** Bourns MF-R020: hold 0.20 A, trip 0.40 A, 60 V. Resistance a few
  ohms (rises after a trip). At 60 mA: drop 0.1–0.5 V. Normal load is under
  a third of the hold current, which leaves room for derating at 40–50 °C.
- **D1** 1N5817: V<sub>RRM</sub> 20 V (> 10 V, so reverse polarity is
  blocked), forward drop ≈ 0.3 V at 60 mA (datasheet V<sub>F</sub> curve).
- **VIN** = 10 − (0.1…0.5) − 0.3 ≈ **9.2–9.6 V** at full load; the README
  uses "M1 minus 0.2–0.7 V".
- **D2** P6KE13A: V<sub>RWM</sub> 11.1 V (no conduction at 10 V; leakage
  ≤ 5 µA), V<sub>BR</sub> 12.4–13.7 V, clamps short surges. A sustained
  over-voltage makes it conduct hard so F1 trips (0.4 A).
  **Caveat:** its clamping voltage at high surge current (18.2 V at 33 A)
  is above the AMS1117's 15 V absolute maximum (clone Nanos); genuine Nanos
  use regulators rated to 20 V. A surge of that size from a fan's internal
  10 V supply through F1 and the cable is not expected; this is a residual
  risk, not a calculated margin.
- **C1** 220 µF / 25 V: bulk and hold-up (§2.4). **C2** 100 nF at VIN:
  high-frequency decoupling.

### 2.3 Regulator dissipation (Nano 5 V regulator) — at 10 V input

Worst case taken literally at **10 V on VIN** (ignoring the D1/F1 drops):

P = (V<sub>IN</sub> − 5 V) × I = (10 − 5) × 0.060 = **0.30 W**
(0.28 W at the realistic 9.6 V).

The regulator is an AMS1117-5.0 (clones) or LM1117 (genuine) in SOT-223.
With the Nano's small copper area, θ<sub>JA</sub> ≈ 90–110 °C/W (datasheet
range for minimum pad), so the rise is **27–33 °C**: T<sub>J</sub> ≈ 73 °C
at 40 °C ambient, against a 125 °C maximum. Dropout ≈ 1.1–1.2 V, so it
regulates down to **V<sub>IN</sub> ≈ 6.2 V**.

**Buck vs linear:** a buck at ~85 % efficiency would take about 0.35 W
instead of 0.6 W from the fan, saving ~0.25 W, but adds switching noise next
to the tach input and another module. The linear regulator is within limits,
so it was kept (FAQ). Above ~100 mA or 12 V input a buck would be preferable.

**U2** MCP1700-3302E: (5 − 3.3) × 0.020 = 34 mW; V<sub>IN</sub> max 6 V
(> 5 V rail); dropout 178 mV typ at 250 mA; needs ≥ 1 µF ceramic in and out
(C3, C4).

### 2.4 Hold-up and brown-out

- D1 stops C1 discharging back into the fan's supply when it dips.
- Time C1 can hold the regulator in regulation at 60 mA, from 9.6 V down to
  6.2 V: t = C × ΔV / I = 220 µF × 3.4 V / 0.060 A = **12.5 ms**.
- If the 5 V rail does collapse, the ATmega's brown-out detector (Nano
  fuses: extended 0xFD = BOD 2.7 V) holds it in reset; D9 floats and R4
  keeps Q1 on while the rail is above Q1's threshold (§2.6). Below about 3 V
  on the 5 V rail Q1 can't be held on and the fan line is released: this
  requires the fan's own 10 V to have collapsed below ~4 V (bench A13).

### 2.5 Reverse polarity and transients summary

| Event | Result |
|---|---|
| J1 +10 V / GND swapped | D1 blocks (20 V rating). No current. |
| Spike on +10 V | D2 clamps; C1/C2 absorb short ones |
| Sustained over-voltage > 12.4 V | D2 conducts, F1 trips at ≥ 0.4 A, resets when it cools |
| Controller short | F1 limits to < 0.4 A sustained |

### 2.6 Speed output: Q1 2N7000 (open drain)

| Quantity | Value | Basis |
|---|---|---|
| Gate drive | V<sub>GS</sub> = 5 V (D9 high, or R4 during reset) | ATmega V<sub>OH</sub> ≥ 4.2 V at 20 mA; static gate current ≈ 0 |
| Threshold | V<sub>GS(th)</sub> 0.8–3.0 V | onsemi 2N7000 datasheet |
| On-resistance | R<sub>DS(on)</sub> ≤ 5.3 Ω at V<sub>GS</sub> 4.5 V | onsemi datasheet |
| Line current | I = V<sub>pull-up</sub> / R<sub>pull-up</sub> ≤ 10 V / 1 kΩ = 10 mA (M3b measures R) | |
| Line "low" | V<sub>DS</sub> ≤ 10 mA × 5.3 Ω = **53 mV** | "Stopped" level |
| Q1 dissipation | ≤ 0.5 mW | |
| D9 peak current | 5 V / (220 Ω + ~25 Ω pin) ≈ **20 mA** < 40 mA abs. max | R3 |
| Gate time constant | 245 Ω × C<sub>iss</sub> ≤ 60 pF ≈ **15 ns** | ≪ 250 µs period at 4 kHz |
| R4 current while D9 low | 5 V / 10 kΩ = 0.5 mA | |
| R5 (if fitted) | 10 V / 4.7 kΩ = 2.1 mA, 21 mW | |
| Line duty (inverted) | gate duty = 1000 ‰ − line duty | `gateDriveFor` |
| PWM | Timer1 mode 14, TOP = 16 MHz / 4 kHz − 1 = 3999 → **4000 Hz, 4000 steps** | Sim checks ICR1 = 3999 |
| Line rise time | τ = R<sub>pull-up</sub> × C<sub>line</sub>; e.g. 100 kΩ × 300 pF = 30 µs (12 % of a period) | Unknown R: M3b |

### 2.7 Tach input (5 V pull-up, series resistor, clamps)

ATmega328P at 5 V: **V<sub>IL</sub> max = 0.3 V<sub>CC</sub> = 1.5 V,
V<sub>IH</sub> min = 0.6 V<sub>CC</sub> = 3.0 V**; inputs have Schmitt
triggers.

| Fan's tach pin state | D2 voltage | Margin |
|---|---|---|
| Open collector sinking (V<sub>CE(sat)</sub> ≤ 0.4 V) | 0.4 + 4.6 × 1k/11k = **0.82 V** | 0.68 V below V<sub>IL</sub> |
| Open collector released | 5.0 V through R6 | 2.0 V above V<sub>IH</sub> |
| Push-pull 3.3 V high | 3.3 + 1.7 × 1k/11k = **3.45 V** | 0.45 V above V<sub>IH</sub> |
| Push-pull 10–12 V high | clamped by D3 to ≈ 5.3 V; current (12 − 5.3) / 1 kΩ = **6.7 mA** | BAT85 I<sub>F</sub> max 200 mA; the 5 V rail's ≥ 25 mA load absorbs it |
| Negative spike | clamped by D4 to ≈ −0.3 V | |
| Nothing connected | 5.0 V; no edges; NO TACH after 8 s | |

- Filter: R6‖R7 ≈ 0.91 kΩ × 10 nF ≈ **9 µs** on falling edges; release
  (rising) through R6: 10 kΩ × 10 nF = 100 µs, reaching V<sub>IH</sub> in
  ≈ 92 µs. Tach periods are milliseconds (up to 1 kHz is accepted:
  `TACH_MIN_PERIOD_US` 1000 = 30 000 RPM at 2 PPR).
- Firmware: falling edges on INT0, glitch rejection < 1 ms, RPM from period
  over a 500 ms window, 0 RPM after 1.5 s without edges.

### 2.8 I²C and module inputs

- R1/R2 2.2 kΩ to 3.3 V: low-level sink 1.5 mA (I²C limit 3 mA). Rise time
  with ~100 pF of bus: 0.85 × 2.2 kΩ × 100 pF ≈ **190 ns** < 300 ns
  (fast-mode limit at 400 kHz). The original 4.7 kΩ gave ≈ 400 ns: changed
  (§7, finding S-1).
- 3.3 V high at the ATmega: 3.3 V ≥ V<sub>IH</sub> 3.0 V, margin 0.3 V.
- `Wire.begin()` enables the AVR's internal 5 V pull-ups (20–50 kΩ); the
  firmware switches them off right after `oled.begin()`. While both are on,
  the line idles at 3.3 + 1.7 × 2.2/(2.2 + 35) ≈ 3.4 V, within the SH1106's
  V<sub>DD</sub> + 0.3 V input limit.
- Encoder/keys: switches to GND with the AVR's 5 V internal pull-ups; they
  do not connect to the SH1106.

### 2.9 Timing and CPU load

- Display: page buffer, 4 passes per frame, 1 KB at 400 kHz ≈ 25–30 ms per
  frame (estimate). 10 Hz refresh ≈ 30 % of the time. Encoder and tach are
  interrupt-driven, so no edges are lost during transfers.
- Worst `loop()` pass ≈ one frame (< 50 ms) ≪ 2 s watchdog.

### 2.10 Logic level summary

| Signal | Driver | Receiver | Levels |
|---|---|---|---|
| D9 → Q1 gate | ATmega 0/5 V | 2N7000 V<sub>th</sub> ≤ 3 V | 5 V ≥ 3 V + margin |
| Q1 drain → fan | open drain | fan input (pull-up to ~10 V) | 0.05 V / released |
| Fan tach → D2 | OC or push-pull | ATmega Schmitt input | ≤ 0.82 V / ≥ 3.45 V |
| SDA/SCL | open drain, 3.3 V pull-ups | ATmega, SH1106 | 0 / 3.3 V |
| Encoder/keys → D3–D7 | switch to GND | ATmega + 5 V internal pull-up | 0 / 5 V |

### 2.11 Memory

| | Used | Limit | Headroom |
|---|---|---|---|
| Flash | 25 404 B | 30 720 B | 5 316 B (17 %) |
| Static RAM | 1 276 B | 2 048 B | 772 B for stack |
| Stack, estimated | ≈ 360 B | | deepest path: `loop` 11 + `updateDisplay` 56 + `drawDiagnostics` 67 + `vfprintf` + U8g2 + one ISR frame (`-fstack-usage`) |
| Stack, measured | min. 570 B never touched | | simavr run, main screen (§8.3) |

The full 1 KB frame buffer was tried first and **does not fit** (2 044 B
static, 4 B left): the firmware uses the 256-byte page buffer
(`OLED_PAGE_BUFFER 1`).

---

## 3. Pin assignment cross-check

`python3 test/check_pins.py` (also run by `make -C test`) parses
`config.h`, `hardware/schematic.py` (Nano pin labels, physical numbers and
the wires to the module), `hardware/wiring.py`, PINOUT.md and README.md, and
fails on any disagreement. A deliberate mutation (D4 → 8 in `config.h`) was
detected. Result: **72 checks, 0 mismatches** (§8.5).

| Signal | Nano pin (physical) | config.h | PINOUT.md | Schematic | Wiring view | README |
|---|---|---|---|---|---|---|
| Tach | D2 / INT0 (5) | `PIN_TACH 2` | ✓ | ✓ D2 / INT0, 5 | ✓ green → D2 | ✓ Step 6 |
| Encoder A | D3 (6) | `PIN_ENC_A 3` | ✓ TRA | ✓ wired to TRA | ✓ purple TRA → D3 | ✓ Step 4 |
| Encoder B | D4 (7) | `PIN_ENC_B 4` | ✓ TRB | ✓ wired to TRB | ✓ grey TRB → D4 | ✓ Step 4 |
| Push switch | D5 (8) | `PIN_ENC_SW 5` | ✓ PSH | ✓ wired to PSH | ✓ cyan PSH → D5 | ✓ Step 4 |
| CON key | D6 (9) | `PIN_KEY_CON 6` | ✓ CON | ✓ wired to CON | ✓ light blue → D6 | ✓ Step 4 |
| BAK key | D7 (10) | `PIN_KEY_BAK 7` | ✓ BAK | ✓ wired to BAK | ✓ violet → D7 | ✓ Step 4 |
| Fan PWM | D9 / OC1A (12) | `PIN_FAN_PWM 9` | ✓ | ✓ D9 / OC1A, 12 | ✓ brown → D9 | ✓ Step 6 |
| Status LED | D13 (16) | `PIN_STATUS_LED 13` | ✓ | on-board (note) | — | — |
| SDA | A4 (23) | fixed TWI | ✓ | ✓ A4 / SDA, 23 | ✓ blue → A4 | ✓ Step 4 |
| SCL | A5 (24) | fixed TWI | ✓ | ✓ A5 / SCL, 24 | ✓ white → A5 | ✓ Step 4 |
| VIN | VIN (30) | — | ✓ | ✓ 30 | ✓ red → VIN | ✓ Step 2 |
| +5V | 5V (27) | — | ✓ | ✓ 27 | ✓ orange → 5V | ✓ Step 4 |
| GND | GND (29, 4) | — | ✓ | ✓ 29, 4 | ✓ black → GND | ✓ Step 2 |
| J1 pins | 4 +10V, 3 GND, 2 PWM, 1 TACH | — | ✓ | ✓ | ✓ | ✓ M1–M5 |

D8 (`PIN_SIM_TACH_OUT`) is used only in the Wokwi build and appears only in
PINOUT.md, config.h and `sim/diagram.json`.

---

## 4. Verified versus assumed

### Verified

| What | How |
|---|---|
| Firmware compiles for the Nano (both processor settings), no warnings from project files | arduino-cli 1.1.1, ArduinoCore-avr 1.8.6, avr-gcc 7.3.0, U8g2 2.35.30 (§8.4) |
| Flash and RAM fit, with stack headroom | size report; `-fstack-usage`; simavr stack paint (§2.11) |
| No EEPROM code | grep check and `avr-nm` of the linked ELF |
| Logic: encoder, button, duty mapping, inversion, Timer1 values, tach maths, timeout, % and guards, controller state machine, calibration, serial parser | 1 107 unit checks, C++11 and C++17 (§8.1) |
| Whole sketch behaviour, screen content and layout, power-up state, millis() wrap | 6 simulation scenarios, ~1 400 checks, every frame layout-checked (§8.2) |
| Real AVR binary boots, prints status, keeps the fan at 0 %, watchdog interrupt recovery with fan forced off | simavr (§8.3) |
| Pin numbers agree in all documents | `check_pins.py` (§3) |
| Watchdog second-timeout reset when the interrupt can't run | ATmega328P datasheet (WDTCSR bit WDIE); **not** reproducible in simavr |
| Circuit values against part limits | §2 calculations from datasheet figures |

### Assumed: must be confirmed on hardware

| Assumption | Confirmed by |
|---|---|
| J1 pin functions (4 = +10 V, 3 = GND, 2 = PWM, 1 = tach?) | M1, M3–M5 |
| The port supplies ≥ 60 mA without sagging | M2, bench B1, B13 |
| Line low = stop, released = full; open-drain at 4 kHz works | M4, bench B3, B4, B7 |
| A tach exists on pin 1, its type and pulses per revolution | M5, bench B5 (no published source) |
| Module is SH1106, I²C 0x3C, works on 3.3 V | build step 5 |
| EC11 is 20 pulses / 20 detents (4 edges per click) | build step 5 |
| The real display has no visible flicker with the page buffer | bench A14 |
| Brown-out behaviour with the real fan supply | bench A13, B10 |
| Thermal behaviour in the enclosure | bench B13 |
| Wokwi diagram runs as described | not run (sim/README) |

---

## 5. Hardware assumptions: evidence and status

Each assumption, the best evidence found, and the builder's check. Sources
are linked in the README *Sources* section. "Builder report" means a hobbyist
forum post or repository, not a manufacturer document.

### H1. Connector and pinout

- **Assumption:** the control port is a PC-peripheral-style 4-pin "Molex"
  (8981 family, 5.08 mm), **not** a 4-pin PC fan header. Pin 4 = +10 V,
  3 = GND, 2 = PWM (speed), 1 = tach (unconfirmed), numbered left to right
  with the chamfered corners up, looking at the fan lead.
- **Evidence:** RollItUp T6/Hyperfan thread (pin 4 = +10 VDC, 3 = GND,
  2 = PWM, 1 = "not used"; chamfer up, left to right); chriskoz/AutoFan for
  the S6 ("4-pin molex ATX HDD connector", "10V / GND / PWM", 4th pin N/C);
  RollItUp ESP32 guide ("same Molex connector used in PC's power supply to
  disks"); AC Infinity manual (fan has "a molex connector"). The 5.08 mm
  pitch follows from the connector family, not from a source.
- **Caveat:** newer S6 units and the PRO line use a USB-C ("UIS") port with
  a different pinout (coofercat/ha-ac-infinity-fan).
- **Check:** M1 (supply and ground, mirror numbering if negative), M3, M4,
  M5. Wire colours are not documented: identify by measuring.

### H2. Supply voltage and current

- **Assumption:** ~10 V DC on pin 4; enough current for ~60 mA.
- **Evidence:** voltage: several builder reports ("10V source ... provided
  by the CL itself", "the fan's 10V instead of 12V ... enough to power the
  Shelly and a few LEDs"). Current: **no source**; one builder reports
  trouble when several channels were powered from it.
- **Check:** M1 (9.5–10.5 V), M2 (≤ 0.5 V drop at 45 mA). Fallback: separate
  adapter (FAQ).

### H3. Control signal

- **Assumption:** PWM by pulling an internally pulled-up ~10 V line to
  ground (open drain); not 0–10 V analogue. Stock controller ≈ 4 kHz.
- **Evidence:** chriskoz ("controlled by grounding and opening the 10V
  control wire", period ~245 µs); ESPHome builds with `inverted: true` and a
  MOSFET to GND at 1 kHz; coofercat (~5 kHz on the USB-C model).
- **If it is actually 0–10 V analogue:** an analogue input would respond to
  the average voltage, which the open-drain PWM also provides; if a fan
  needed a smooth DC level, an RC filter (e.g. 10 kΩ + 10 µF) after Q1's
  drain would be added. Detect: M4 behaves the same; bench A7 levels.
- **Check:** M3, M4.

### H4. Internal pull-up and floating behaviour

- **Assumption:** pulled up inside the fan to about its supply voltage; the
  fan runs at full speed when the line floats.
- **Evidence:** chriskoz ("pulled up to 10V by the fan"). Floating = full
  speed is an **inference** (released line = 100 % duty), not a report.
- **Check:** M3 (voltage), M3b (strength), M4 (released = full). If it
  floats, fit R5.

### H5. Does 0 % stop the fan?

- **Assumption:** line held low = stopped.
- **Evidence:** chriskoz (stock "speed 0" = 0 µs high time); coofercat
  (speed 0 → 0 tach pulses, USB-C model).
- **Check:** M4, bench B7. If not: OFF = minimum speed; options in FAQ.

### H6. Tach output

- **Assumption:** none made. The input accepts open collector or push-pull,
  0–12 V, any pulses per revolution; default 2 PPR only scales the RPM
  number.
- **Evidence:** **no source found** for a tach on the 4-pin port (one
  builder lists pin 1 as "not used"); the USB-C model has a tach on D+.
- **Check:** M5, bench B5. Without a tach: RPM 0, ACT `--%`, NO TACH.

### H7. OLED module

- **Assumption:** SH1106, I²C (not SPI), address 0x3C, 3.3 V supply,
  EC11 20 pulses / 20 detents, keys switch to GND, pin order varies.
- **Evidence:** AITRIP listing/manual (manuals.plus, ASIN B0F87DZ51P),
  makerselectronics and bitsandparts listings. Some generic manuals say
  SSD1306 or SH1116, or "3.3–5 V".
- **Check:** build step 5 (screen correct; else `OLED_SSD1306 1` or address
  0x3D; encoder direction and edges per detent).

---

## 6. Known limitations and failure modes

| Situation | What happens | Mitigation |
|---|---|---|
| Mains power cut / fan unplugged | Controller loses power; comes back **OFF at 50 %**. **The fan does not restart by itself.** | By requirement. Change deliberately if automatic restart is needed. |
| Controller's 5 V fails while the fan has power | Q1 off, line released; fan most likely **full speed** (H4). | "Fail-ventilating". A fail-off design would need a normally-on pull-down powered from the fan. |
| Speed wire breaks | Line floats → likely full speed; display unaware unless the tach shows it. | Strain relief, cable gland. |
| Tach wire breaks / no tach | RPM 0; `NO TACH` after 8 s; fan still controlled. | — |
| Program hangs in `loop()` | Watchdog interrupt after 2 s: fan stopped, sketch restarts OFF. | Verified in simavr. |
| Program hangs with interrupts disabled | Second timeout (4 s) → hardware reset; fan held off by R4. Old-bootloader Nanos may then reset-loop until power-cycled (fan stays off). | Datasheet behaviour; bench W2. |
| Supply dips < 6.2 V for > ~12 ms | 5 V sags; brown-out reset possible; restarts OFF. | C1 hold-up; bench B10. |
| Fan 10 V collapses below ~4 V | Q1 can't be held on; line may rise. Fan's own electronics are also out of spec. | — |
| Fan doesn't stop at 0 % | OFF = minimum speed. | Switch mains (FAQ). |
| Low settings don't start the fan | Fan stalls; NO TACH. | `FAN_DUTY_MIN_PCT` (bench B6). |
| Calibration lost at power-off | ACT `--%` until recalibrated. | Copy `TACH_MAX_RPM` into `config.h`. |
| Wrong `TACH_PULSES_PER_REV` | RPM figure off by a factor; ACT % unaffected. | Optical tachometer (FAQ). |
| `millis()` wrap (49.7 days) | Handled (tested). | — |
| I²C bus stuck (display fault, bad wiring) | The core's `Wire` timeout is **off by default**; the firmware turns it on (25 ms, bus reset), so transfers give up instead of blocking. If anything still blocked `loop()`, the watchdog would stop the fan and restart. A hang inside `setup()` (before the watchdog starts) leaves the fan stopped, because D9 is already driven to "stopped" first. | — |
| Upload or serial-monitor open | Nano resets → fan stops, controller restarts OFF. | Expected (FAQ). |
| Large surge above D2's clamp capability | Could exceed an AMS1117's 15 V absolute maximum. | Residual risk (§2.2). |
| Open-loop control | The knob sets duty, not RPM; ACT shows the result. | Possible extension. |

---

## 7. Independent review: findings and fixes

(Filled in after the independent review.)

---

## 8. Recorded test and build output

(Filled in with the final run.)
