# Pinouts and connections

Every connection in the controller, grouped by part, plus the power rails and
a point-to-point wiring list. The graphical version is
[`schematic.svg`](schematic.svg).

> **The fan connector pin functions come from hobbyist reverse-engineering,
> not AC Infinity documentation.** Do the measurements in
> [VERIFICATION.md](VERIFICATION.md) before plugging the controller into your
> fan.

---

## 1. Power rails

| Rail | Nominal | Source | Feeds | Typical load |
|---|---|---|---|---|
| **+10V_FAN** | ~10 V DC | Fan, J1 pin 4 | F1 → D1 | 45–60 mA total |
| **+10V_P** (protected) | ~9.6–9.8 V | After F1 (PTC) and D1 (Schottky, ~0.3 V drop) | Nano **VIN**, R5 (if fitted), R6 tach pull-up | same |
| **+5V** | 5.0 V | Nano on-board regulator (VIN → 5 V) | Nano itself, U2 input, R4 gate pull-up, R9 tach pull-up | ~25 mA plus the OLED |
| **+3V3** | 3.3 V | U2 MCP1700-3302E (from +5V) | OLED module VCC, I²C pull-ups R1/R2 | 10–30 mA (OLED, depends on lit pixels) |
| **GND** | 0 V | Fan, J1 pin 3 | Everything. One common ground. | — |

**Why there are three rails:**

- The Nano's VIN regulator needs at least about 6.5 V. The fan's ~10 V is in
  range, and the regulator only dissipates about 0.25 W at this load, so no
  external buck converter is needed.
- The OLED module is specified for **3.3 V**. U2 supplies it, rather than the
  Nano's `3V3` pin. On most CH340-based Nano clones that pin comes from the
  USB chip's internal regulator and is not meant to power anything.

**Power budget at +10V_FAN:**

| Load | Current |
|---|---|
| ATmega328P at 16 MHz, CH340, power LED | ~20–25 mA |
| OLED at about 25 % of pixels lit (these screens) | ~10–15 mA |
| D13 status LED (while ON) | ~3 mA |
| R6 tach pull-up (tach low) | ≤1 mA |
| R4 / R9 pull-ups, I²C pull-ups | ≤1.5 mA |
| **Total** | **~45–60 mA** (~0.5 W) |

---

## 2. J1: fan connector (4-pin Molex, 5.08 mm / 0.2″ pitch)

This is the same connector family as the old PC "Molex" drive-power plug.
J1 is the half that mates with the cable coming out of the fan, i.e. where the
stock AC Infinity controller plugged in. The easiest source is a 4-pin Molex
extension cable cut in half: keep the end that plugs into the fan's lead.

| J1 pin | Function (reported) | Direction | Level | Connects to |
|---|---|---|---|---|
| **4** | +10 V supply from the fan | fan → controller | ~10 V DC | F1 → D1 → **+10V_P** |
| **3** | Ground | — | 0 V | **GND** |
| **2** | Speed control ("PWM") | controller → fan | Pulled up inside the fan to about 10 V. Q1 pulls it to 0 V. | **Q1 drain** (and R5 if fitted) |
| **1** | Tach / FG (speed feedback) | fan → controller | Pulse train, frequency proportional to RPM | **R6** (pull-up) and **R7** (to Q2 base) |

Pin numbers follow the convention used in the field report this design is
based on: connector held with the chamfered corners up, pins numbered 1–4
from left to right. **Wire colours are not reliable** (an extension cable has
yellow/black/black/red no matter what each pin does). Identify the pins by
measurement and mark them.

Speed line behaviour, as reported for Cloudline EC fans:

- Held at 0 V: fan stopped.
- Left floating / high: full speed.
- PWM between the two: proportional speed. The fan responds to the average
  high time.
- A 3.3 V GPIO driving the line directly only reaches about half speed. That
  is why the line is driven open-drain and allowed to rise to the fan's own
  ~10 V.

---

## 3. U1: Arduino Nano (ATmega328P, 5 V, 16 MHz)

Physical pin numbers: pin 1 is D1/TX, at the end of the digital header
furthest from the USB connector. Pins 1–15 run along that header towards the
USB end, finishing at D12. Pin 16 (D13) sits next to the USB connector on the
other header, and the numbering runs back along it to pin 30 (VIN). The pin
labels are printed on the board.

| Nano pin | Name | Used as | Connects to | Mode |
|---|---|---|---|---|
| 30 | **VIN** | Power in | **+10V_P** | — |
| 29 | GND | Ground | GND | — |
| 4 | GND | Ground | GND | — |
| 27 | **+5V** | Power out (on-board regulator) | U2 VIN, R4, R9 | — |
| 17 | 3V3 | **not used** (see §1) | — | — |
| 5 | **D2** / INT0 | Tach input | Q2 collector, R9 to +5V, C5 to GND | `INPUT`, falling-edge interrupt |
| 6 | **D3** / PCINT19 | Encoder A | U3 **TRA** | `INPUT_PULLUP`, pin-change interrupt |
| 7 | **D4** / PCINT20 | Encoder B | U3 **TRB** | `INPUT_PULLUP`, pin-change interrupt |
| 8 | **D5** | Encoder push switch | U3 **PSH** | `INPUT_PULLUP` (active LOW) |
| 9 | **D6** | CON / KEY1 key *(optional)* | U3 **CON** | `INPUT_PULLUP` (active LOW) |
| 10 | **D7** | BAK / KEY0 key *(optional)* | U3 **BAK** | `INPUT_PULLUP` (active LOW) |
| 12 | **D9** / OC1A | Fan PWM (Timer1) | R3 → Q1 gate; R4 to +5V | `OUTPUT`, 1 kHz hardware PWM |
| 16 | D13 | Status LED (on-board) | — | `OUTPUT`, lit = fan ON |
| 23 | **A4** / SDA | I²C data | U3 **SDA**, R1 to +3V3 | Hardware TWI, internal pull-up **off** |
| 24 | **A5** / SCL | I²C clock | U3 **SCL**, R2 to +3V3 | Hardware TWI, internal pull-up **off** |
| 1, 2 | D1/TX, D0/RX | USB serial (programming) | — | leave free |
| 3, 28 | RESET | — | — | leave free |
| others | D8, D10–D12, A0–A3, A6, A7, AREF | spare | — | — |

All assignments are in [`firmware/cloudline_controller/config.h`](../firmware/cloudline_controller/config.h).
D9 is fixed, because the hardware PWM comes from Timer1 / OC1A. D2 should
stay on INT0. D3/D4 must stay together on port D (PCINT2) for the encoder
interrupt.

---

## 4. U3: AITRIP 1.3″ OLED + EC11 encoder module

SH1106 controller, 128×64, I²C at address **0x3C**, supply **3.3 V**. 20
pulses / 20 detents per turn, push switch on the shaft, plus two extra keys.

> **Wire by the printed labels.** Listings for this module show different pin
> orders (for example `VCC GND SDA SCL TRA TRB PSH CON BAK` and
> `CON SDA SCL PSH TRA TRB BAK GND VCC`). Your board's silkscreen is the only
> authority.

| Module label | Function | Connects to | Notes |
|---|---|---|---|
| **VCC** | Supply | **+3V3** (U2 output) | Do not feed 5 V unless you have confirmed the board has its own regulator (see VERIFICATION.md B4) |
| **GND** | Ground | GND | — |
| **SDA** | I²C data | Nano A4 | R1 4.7 kΩ to +3V3 |
| **SCL** | I²C clock | Nano A5 | R2 4.7 kΩ to +3V3 |
| **TRA** | Encoder phase A | Nano D3 | Switches to GND |
| **TRB** | Encoder phase B | Nano D4 | Switches to GND |
| **PSH** | Encoder push button | Nano D5 | Switches to GND: short press = fan ON/OFF, 3 s hold = calibrate |
| **CON** (KEY1) | Extra key "confirm" | Nano D6 | Optional: diagnostics page |
| **BAK** (KEY0) | Extra key "back" | Nano D7 | Optional: back / cancel calibration |

The keys and encoder contacts only ever pull to GND. The ATmega's internal
pull-ups to 5 V are enabled on D3–D7. If the module has its own pull-ups to
its 3.3 V VCC, the two simply share: a few tens of µA flow into the 3.3 V
rail, which the OLED's own load absorbs. Those pins do not connect to the
SH1106, so nothing on the display sees 5 V. The I²C lines are different
(see the design notes, §4).

---

## 5. Discrete parts: pinouts and connections

### Transistors (TO-92, flat face towards you, legs down, left to right)

| Ref | Part | Left | Middle | Right | Connections |
|---|---|---|---|---|---|
| **Q1** | 2N7000 N-MOSFET | **S**ource | **G**ate | **D**rain | S → GND · G → R3 (to D9) and R4 (to +5V) · D → J1 pin 2 |
| **Q2** | 2N3904 NPN | **E**mitter | **B**ase | **C**ollector | E → GND · B → R7 (from J1 pin 1) and R8 (to GND) · C → Nano D2, R9 (to +5V), C5 |

> Substitutes may have different pin orders. **BS170 is D-G-S** (reversed
> from the 2N7000). **BC547 is C-B-E.** Check the datasheet of the part you
> actually fit.

### U2: MCP1700-3302E/TO (3.3 V LDO, TO-92)

| Pin (flat face towards you, legs down, left to right) | Function | Connects to |
|---|---|---|
| 1 | GND | GND |
| 2 | VIN | +5V (Nano 5V pin), C3 1 µF to GND |
| 3 | VOUT | +3V3, C4 1 µF to GND |

Confirm against the datasheet for your package (the SOT-23 version has a
different order). Any 3.3 V LDO that accepts 5 V in and gives at least
100 mA works, for example an AMS1117-3.3 breakout or an HT7333. Keep the
input/output capacitors its datasheet asks for.

### Diodes

| Ref | Part | Anode (no band) | Cathode (band) |
|---|---|---|---|
| D1 | 1N5817 Schottky | F1 (from J1 pin 4) | +10V_P rail |
| D2 | P6KE13A TVS (unidirectional) | GND | +10V_P rail |

### Resistors and capacitors

| Ref | Value | Between | Purpose |
|---|---|---|---|
| R1 | 4.7 kΩ | SDA ↔ +3V3 | I²C pull-up (3.3 V logic) |
| R2 | 4.7 kΩ | SCL ↔ +3V3 | I²C pull-up |
| R3 | 220 Ω | D9 ↔ Q1 gate | Gate series resistor: keeps the pin's peak gate-charge current near 20 mA, and damps ringing |
| R4 | 10 kΩ | D9/Q1 gate ↔ +5V | Holds Q1 on (fan **OFF**) while D9 is high-impedance at reset/boot |
| R5 | 4.7 kΩ **DNP** | J1 pin 2 ↔ +10V_P | Only if the fan's speed input has no internal pull-up |
| R6 | 10 kΩ | J1 pin 1 ↔ +10V_P | Tach pull-up (open-collector tach outputs) |
| R7 | 10 kΩ | J1 pin 1 ↔ Q2 base | Base current limit / input protection |
| R8 | 10 kΩ | Q2 base ↔ GND | Keeps Q2 firmly off when the tach is low (even a weak 0.8 V low) or floating |
| R9 | 10 kΩ | Q2 collector (D2) ↔ +5V | Collector pull-up = 5 V logic level for D2 |
| C1 | 100 µF ≥25 V electrolytic | +10V_P ↔ GND | Bulk reservoir (+ to the rail) |
| C2 | 100 nF ceramic | +10V_P ↔ GND | High-frequency decoupling, mount close to the Nano VIN |
| C3 | 1 µF ceramic | U2 VIN ↔ GND | LDO input capacitor |
| C4 | 1 µF ceramic | U2 VOUT ↔ GND | LDO output capacitor (needed for stability) |
| C5 | 1 nF ceramic | D2 ↔ GND | Tach noise filter (τ = 10 µs, so up to ~10 kHz) |
| F1 | PTC 0.2 A hold / 0.4 A trip (e.g. Bourns MF-R020) | J1 pin 4 ↔ D1 anode | Protects the fan's supply if the controller shorts |

---

## 6. Point-to-point wiring list

For perfboard or protoboard builds. Tick each line off as you solder it.

| # | From | To | Net |
|---|---|---|---|
| 1 | J1 pin 4 | F1 lead 1 | +10V_FAN |
| 2 | F1 lead 2 | D1 anode | |
| 3 | D1 cathode | Nano VIN (30), C1 +, C2, D2 cathode, R5*, R6 | **+10V_P** |
| 4 | J1 pin 3 | Nano GND (29 and 4), C1 −, C2, D2 anode, Q1 S, Q2 E, R8, C3, C4, C5, U2 GND, U3 GND | **GND** |
| 5 | J1 pin 2 | Q1 drain, R5* | FAN_PWM |
| 6 | Nano D9 (12) | R3, R4 | PWM_GATE |
| 7 | R3 other end | Q1 gate | |
| 8 | R4 other end | +5V | |
| 9 | J1 pin 1 | R6 other end, R7 | TACH_IN |
| 10 | R7 other end | Q2 base, R8 other end | |
| 11 | Q2 collector | Nano D2 (5), R9, C5 | TACH_5V |
| 12 | R9 other end | +5V | |
| 13 | Nano +5V (27) | U2 VIN, C3, R4, R9 | **+5V** |
| 14 | U2 VOUT | U3 VCC, C4, R1, R2 | **+3V3** |
| 15 | Nano A4 (23) | U3 SDA, R1 other end | SDA |
| 16 | Nano A5 (24) | U3 SCL, R2 other end | SCL |
| 17 | Nano D3 (6) | U3 TRA | ENC_A |
| 18 | Nano D4 (7) | U3 TRB | ENC_B |
| 19 | Nano D5 (8) | U3 PSH | ENC_SW |
| 20 | Nano D6 (9) | U3 CON | KEY_CON (optional) |
| 21 | Nano D7 (10) | U3 BAK | KEY_BAK (optional) |

\* R5 only if [VERIFICATION.md](VERIFICATION.md) step A5 says so.

**Layout tips:**

- Put C2 right at the Nano's VIN/GND pins.
- Keep Q1 and the J1 wiring away from the I²C wires.
- Use a socket for the Nano, so you can do the bench checks in
  VERIFICATION.md part B before anything is plugged into the fan.
