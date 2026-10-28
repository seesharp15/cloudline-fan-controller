# Bench test procedure

Software tests can't check anything electrical. This procedure checks every
requirement on real hardware: **Part A** with a bench supply and a dummy load
standing in for the fan, then **Part B** with the real fan. Run it after the
build steps in the README (or after any hardware change) and fill in the
result column.

Requirement IDs (R1–R9 from the brief, D1–D12 from the design list) refer to the traceability matrix in
[VERIFICATION.md §1](VERIFICATION.md#1-requirements-traceability-matrix).

---

## Equipment

| Item | Used for |
|---|---|
| Multimeter (DC V, Ω, ideally Hz) | All tests |
| Bench supply 0–12 V with current limit, **or** a 9 V battery | Part A power. A battery can't do tests A3 and A13. |
| 4.7 kΩ resistor | "Fan pull-up" for the dummy load |
| LED (any colour) + 1 kΩ resistor | Visual speed indicator on the dummy load |
| Second Arduino (any 5 V board) **or** a signal generator (5 V square wave) | Fake tach signal. Without one, tests A9/A10 can only be done with the manual method. |
| Computer with the Arduino IDE | Serial console tests and the watchdog test builds |

### Dummy load ("fan emulator")

Wire this to J1's loose ends instead of the fan:

```
 Bench supply +10.0 V ──┬──────────────► J1 "+10V" wire
                        │
                       4.7 kΩ   (stands in for the fan's internal pull-up)
                        │
                        ├──────────────► J1 "PWM" wire     ← measure here: "LINE"
                        │
                       1 kΩ
                        │
                       LED (anode up, cathode down)
                        │
 Bench supply GND ──────┴──────────────► J1 "GND" wire

 Fake tach: second Arduino pin 8 ───────► J1 "TACH" wire
            second Arduino GND  ───────► J1 "GND" wire
```

The LED lights when the line is released (fan "fast") and goes dark when Q1
pulls it low (fan "stopped"); its brightness follows the speed setting.
With the LED connected, LINE reads about 5.5 V when fully released; for the
precise voltage table in A7, disconnect the LED branch.

Fake tach sketch for the second Arduino (2 pulses per revolution, so
50 Hz = 1500 RPM and 80 Hz = 2400 RPM; `tone()` can't go below 31 Hz):

```cpp
// Upload to the SECOND Arduino. Change TACH_HZ and re-upload as the tests ask.
const unsigned TACH_HZ = 50;   // 0 = no tach signal
void setup() { if (TACH_HZ) tone(8, TACH_HZ); }
void loop() {}
```

Manual method without a second board: briefly touch the J1 TACH wire to
GND; each touch counts as one or more pulses on the diagnostics page (`n=`).

---

## Part A: bench supply and dummy load

Set the bench supply to **10.0 V, current limit 150 mA**, output off. USB
**disconnected** unless a test says otherwise.

| ID | Requirement | Procedure | Expected result | Result |
|---|---|---|---|---|
| **A1** | R6, D5 Powered from 10 V with conditioning | Output on. Measure socket VIN, Nano 5V, module VCC, all to GND. Read the supply current. | VIN **9.3–9.8 V**; 5V **4.9–5.1 V**; VCC **3.25–3.35 V**; current **35–60 mA** (LED branch adds ~5 mA when lit) | |
| **A2** | D5 Reverse polarity | Output off. Swap the supply leads at J1. Output on for 5 s. Output off, swap back, output on. | While reversed: current **< 1 mA**, display dark. Afterwards the controller works normally. | |
| **A3** | D5 Over-voltage clamp (bench supply only) | Raise the supply slowly to 13.0 V, hold 5 s, back to 10 V. | Current rises noticeably above ~12.4 V (D2 starts to conduct) but stays below 150 mA; **F1 may trip** above ~13 V (current falls to tens of mA and recovers after cooling). No damage; normal operation at 10 V. | |
| **A4** | R3, D2 Fan off at power-up | Output off. Meter on LINE. Output on and watch the LED and the meter. Repeat 10 times. | LINE **< 0.1 V** once the supply is up; no visible LED flash (a sub-millisecond pulse while the supply rises can't be seen and is too short for a fan). Display `FAN OFF`, SET 50 %. | |
| **A5** | D2 Off during reset / bootloader | Fan ON at 100 % (LED bright). Hold the Nano's **reset button** for 10 s, then release. | While held: LINE **< 0.1 V**, LED dark. After release: display `FAN OFF`, LINE stays < 0.1 V. | |
| **A6** | R1, D6 Encoder controls setting | Turn the knob slowly 10 clicks clockwise, 10 back, then spin quickly. Turn down past 0 and up past 100. | 1 % per slow click (50 → 60 → 50); 5 % per click when spun fast; stops at **0 %** and **100 %**; no skipped or reversed clicks in 10 slow clicks. | |
| **A7** | D3 PWM drive, polarity, level | Disconnect the LED branch. Fan ON. Set 0, 25, 50, 75, 100 % and measure LINE (DC average). | **0 %: < 0.1 V · 25 %: 2.3–2.7 V · 50 %: 4.8–5.2 V · 75 %: 7.3–7.7 V · 100 %: 9.6–10.0 V** (≈ setting × supply). Higher setting = higher voltage. | |
| **A8** | R2, D7 Button toggles | Fan OFF. Press the knob **20 times**, counting. | Exactly **20 toggles** (ends OFF); D13 LED and title bar match every time; no double toggles. | |
| **A9** | R4, D10 Setting persists while powered | Set 37 %. Switch OFF, wait 10 s, switch ON. Then supply off for 10 s, on again. | After OFF/ON: still **37 %**, LINE ≈ 3.6 V. After the power cycle: **OFF, 50 %**. | |
| **A10** | R5, D8 Tach → RPM display | Fake tach at **50 Hz**. Fan ON. | Within 2 s: **1500 RPM** (±1 %) in the title bar; ACT shows `--%` (not calibrated). | |
| **A11** | D8 Tach timeout, NO TACH | With the fan ON and RPM showing, set the fake tach to 0 (or disconnect it). | RPM falls to **0** within **1.5–2 s**; after **8 s** footer `! NO TACH / STALLED`, ACT `--%`. LINE unchanged (the fan keeps being driven). | |
| **A12** | D8 Calibration | Fake tach **80 Hz**. Fan ON, hold the knob 3 s. After `CAL DONE`, set the fake tach to **40 Hz**. | `CALIBRATE` screen, LINE at 100 % during the run; `CAL DONE 100% = 2400 RPM` (±1 %); then ACT shows **50 %** at 40 Hz. Serial prints `#define TACH_MAX_RPM 2400` (±1 %). | |
| **A13** | D5 Brown-out behaviour (bench supply only) | Fan ON at 50 %. Lower the supply slowly from 10 V to 3 V and back, watching the display and LINE. | Works normally down to about **6.5 V**. Below that the display may reset (comes back `FAN OFF`). LINE stays **< 0.5 V** while the Nano is resetting. Below about **4 V** on VIN the 5 V rail falls under ~3 V, Q1 can no longer be held fully on, and LINE may rise: the fan's own supply would have collapsed by then. Record the voltage at which LINE first rises. | |
| **A14** | R5, D9 Display layout and refresh | Watch each screen: OFF, ON, NO TACH, calibrating, result, diagnostics (CON). Turn the knob quickly. | Matches the pictures in the README; **no flicker** or blank frames; the value follows the knob without visible lag (refresh 10 Hz, plus immediately after input). | |
| **A15** | R7 Pinouts | Compare your wiring with PINOUT.md §6 line by line. | Every line ticked. | |
| **A16** | Serial console | USB connected, Serial Monitor 115200, Newline. Send `?`, `s`, `set 30`, `on`, `off`, `c` (while OFF), `m 1800`, `bogus`. | Help text; status line; setting 30 %; fan ON/OFF; `calibration needs the fan ON`; status shows `max=1800`; `? unknown command`. | |
| **A17** | D10 No EEPROM writes | (Software check) `make -C test` | Prints `no-eeprom: firmware has no EEPROM access`. | |

### Watchdog tests (optional, needs a temporary firmware change)

These prove that a hung program leaves the fan stopped. **Undo the change
and re-upload the normal firmware afterwards.**

| ID | Requirement | Procedure | Expected result | Result |
|---|---|---|---|---|
| **W1** | D11 Watchdog, normal hang | Add `#define TEST_HANG_AFTER_MS 15000` as the **first line of `config.h`**, upload. Within 15 s switch the fan ON at 100 % (LED bright). Watch LINE and the Serial Monitor. | At 15 s: `TEST: loop() hangs now`. About **2 s later** LINE drops **< 0.1 V** (LED dark) and the controller restarts: `FAN OFF`, serial `! restarted by the watchdog`. The test hang doesn't repeat after a restart. | |
| **W2** | D11 Watchdog, hang with interrupts off | As W1 but with `#define TEST_HANG_CLI_AFTER_MS 15000`. | About **4 s** after `TEST: hang with interrupts off`, the chip resets (hardware reset). **Optiboot:** it restarts normally, `FAN OFF`. **Old bootloader:** it may stay in a reset loop (display dark, L LED flickering); LINE stays **< 0.1 V** the whole time. Power-cycle to recover. | |

---

## Part B: the real fan

Do Part A first. Disconnect the dummy load, plug J1 into the fan's control
lead, then plug the fan into mains.

| ID | Requirement | Procedure | Expected result | Result |
|---|---|---|---|---|
| **B1** | R3, R6 First power-up | Plug the fan into mains. | Within 1 s: `FAN OFF`, fan **not running** (coasts to a stop if it was turning). Rails as in A1 (VIN = M1 minus 0.2–0.7 V). | |
| **B2** | R3 Off at power-up, repeated | Unplug and replug the fan's mains 5 times, 10 s apart. | Never starts by itself; always `FAN OFF, 50 %`. | |
| **B3** | R2 On/off | Press the knob; wait 20 s; press again. | Runs at 50 % within a few seconds; stops (coasts down) after OFF. | |
| **B4** | R1 Speed range | Fan ON. 100 %, wait 20 s; 50 %; 25 %; 10 %. | Audible/airflow change at each step; RPM rises and falls with the setting. | |
| **B5** | R5 Tach present | Fan ON at 100 %. Diagnostics page (CON). | `n=` counting up; `Tach` frequency steady (record it: ________ Hz). If it stays 0, the port has no tach: everything else must still pass; ACT stays `--%`. | |
| **B6** | Minimum duty | Fan ON, set 1 %. Wait 15 s. If RPM is 0, raise 1 % at a time, waiting 15 s each, until the fan turns. Then lower until it stops. | Record the lowest setting that keeps it turning (________ %) and the setting at which it starts from rest (________ %). If the start value is above ~5 %, set `FAN_DUTY_MIN_PCT` to the start value minus 1, re-upload, and check that 1 % now turns the fan. | |
| **B7** | Does 0 % stop the fan? | Fan ON at 30 %. Set **0 %**. Wait 30 s. Then press OFF. | RPM reaches **0** (the fan stops) at 0 % and when OFF. If it keeps turning slowly, note it: the fan doesn't stop at 0 % (see FAQ). | |
| **B8** | D8 Calibration on the fan | Fan ON, hold the knob 3 s. Keep doors/vents steady. | `CAL DONE` with a plausible top speed (________ RPM). Copy the serial `#define TACH_MAX_RPM` line into `config.h` if you want to keep it. | |
| **B9** | R5 ACT tracks SET | After B8, set 25, 50, 75 %, waiting 20 s each. | ACT reaches a stable value at each step (EC fans are not linear, so ACT need not equal SET). Record: 25 % → ___ % · 50 % → ___ % · 75 % → ___ %. | |
| **B10** | D5 No reset on speed changes | Turn quickly from 0 to 100 % and back 10 times, 5 s apart. Optionally monitor VIN with the meter's min/max function. | No resets (display never jumps back to `FAN OFF` by itself); VIN minimum stays **above 6.5 V**. | |
| **B11** | R4 Persistence with the fan | Set 63 %, OFF, ON. | Back to 63 %. | |
| **B12** | Fan unplugged / replugged | Fan ON at 60 %. Unplug J1 from the fan for 5 s, plug back in. | Display dark while unplugged. After replugging: `FAN OFF, 50 %`, the fan stops. | |
| **B13** | Thermal | Fan ON at 100 % for 1 hour with the controller in its enclosure. | Nano regulator (the 3-pin part near VIN) **warm but touchable** (below ~60 °C); D2, F1, Q1 cool. Display and speed stable. | |
| **B14** | USB at the same time | Connect USB while the fan runs. Open the Serial Monitor (this resets the Nano). | The fan stops during the reset and the controller comes back `FAN OFF`; afterwards ON/OFF and `s` work with USB connected. | |

---

## Sign-off

| | |
|---|---|
| Fan model / serial | |
| Nano type (genuine/clone, processor setting) | |
| OLED driver setting (`OLED_SSD1306`) | |
| Firmware commit | |
| Part A passed (date, initials) | |
| Part B passed (date, initials) | |
| Settings changed in `config.h` | |
