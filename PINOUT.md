# Pinouts and connections

Every connection in the controller, grouped by part, plus the power rails and
a point-to-point wiring list. The drawings are
[hardware/schematic.svg](hardware/schematic.svg) (electrical) and
[hardware/wiring.svg](hardware/wiring.svg) (physical, with wire colours).
Pin numbers in the firmware are in
[firmware/cloudline_controller/config.h](firmware/cloudline_controller/config.h).
[VERIFICATION.md §3](VERIFICATION.md#3-pin-assignment-cross-check) checks that
all four documents agree.

> **The fan connector pin functions come from builder reports, not AC
> Infinity documentation.** Do the measurements in
> [README.md, Pre-connection verification](README.md#pre-connection-verification)
> before plugging the controller into your fan.

---

## 1. Power rails

| Rail | Nominal | Source | Feeds |
|---|---|---|---|
| **+10V_FAN** | ~10 V DC (measured on your fan in check M1) | Fan, J1 pin 4 | F1 |
| **+10V_P** (protected) | ~9.6 V | After F1 (PTC, ~0.1 V) and D1 (Schottky, ~0.3 V) | Nano **VIN**, D2, C1, C2, R5 (only if fitted) |
| **+5V** | 5.0 V | Nano on-board regulator (VIN → 5 V) | Nano, U2 input, R4 (Q1 gate pull-up), R6 (tach pull-up), D3 clamp |
| **+3V3** | 3.3 V | U2 MCP1700-3302E (from +5V) | OLED module VCC, R1/R2 I²C pull-ups |
| **GND** | 0 V | Fan, J1 pin 3 | Everything. One common ground. |

Current budget and regulator calculations:
[VERIFICATION.md §2](VERIFICATION.md#2-electrical-calculations).

---

## 2. J1: fan connector (4-pin "Molex" PC-peripheral type, 5.08 mm pitch)

J1 is the end of a 4-pin Molex extension cable that plugs into the fan's
control lead, where the stock AC Infinity controller used to plug in.

**View shown: the mating face of the fan's lead, looking into the contacts,
with the two chamfered corners at the top. Pins are numbered 1 to 4 from left
to right.** This is the convention of the builder report the pin functions
come from. The mating plug on your cable is the mirror image, so its pin 1
is on the right when you look into it. Do not rely on either picture alone:
identify each pin by measuring (check M1) and label the wires.

```
          ___________________________
         /                           \      fan lead, mating face
        |  (1)     (2)     (3)     (4) |    chamfered corners up
        |__________________________ __|
          TACH?   PWM     GND    +10V
```

| J1 pin | Function (reported) | Direction | Level | Connects to |
|---|---|---|---|---|
| **4** | +10 V supply from the fan | fan → controller | ~10 V DC | F1 → D1 → **+10V_P** |
| **3** | Ground | — | 0 V | **GND** |
| **2** | Speed control ("PWM") | controller → fan | Pulled up inside the fan to ~10 V. Q1 pulls it to 0 V. | **Q1 drain** (and R5 if fitted) |
| **1** | Tach / speed feedback: **unconfirmed**. One builder lists it as "not used". | fan → controller | unknown; the input accepts 0–12 V, open-collector or push-pull | **R7** (1 kΩ) |

Wire colours: the fan side's colours are not documented. A PC extension
cable is usually yellow/black/black/red regardless of what each pin does.

---

## 3. U1: Arduino Nano (ATmega328P, 5 V, 16 MHz)

Physical pin numbers: pin 1 is D1/TX, at the end of the digital header
furthest from the USB connector; pins 1–15 run towards the USB end, ending at
D12. Pin 16 (D13) sits next to the USB connector on the other header, and the
numbering runs back to pin 30 (VIN). The labels are printed on the board.

| Nano pin | Name | Used as | Connects to | Firmware (`config.h`) | Mode |
|---|---|---|---|---|---|
| 30 | **VIN** | Power in | **+10V_P** | — | — |
| 29 | GND | Ground | GND | — | — |
| 4 | GND | Ground | GND | — | — |
| 27 | **+5V** | Power out (on-board regulator) | U2 VIN, R4, R6, D3 cathode | — | — |
| 17 | 3V3 | **not used** | — | — | — |
| 5 | **D2** / INT0 | Tach input | R7, R6, D3 anode, D4 cathode, C5 | `PIN_TACH 2` | `INPUT`, falling-edge interrupt |
| 6 | **D3** / PCINT19 | Encoder A | U3 **TRA** | `PIN_ENC_A 3` | `INPUT_PULLUP`, pin-change interrupt |
| 7 | **D4** / PCINT20 | Encoder B | U3 **TRB** | `PIN_ENC_B 4` | `INPUT_PULLUP`, pin-change interrupt |
| 8 | **D5** | Encoder push switch | U3 **PSH** | `PIN_ENC_SW 5` | `INPUT_PULLUP`, polled, active LOW |
| 9 | **D6** | CON key *(optional)* | U3 **CON** | `PIN_KEY_CON 6` | `INPUT_PULLUP`, polled, active LOW |
| 10 | **D7** | BAK key *(optional)* | U3 **BAK** | `PIN_KEY_BAK 7` | `INPUT_PULLUP`, polled, active LOW |
| 11 | D8 | Wokwi simulator only (fake tach out) | nothing on hardware | `PIN_SIM_TACH_OUT 8` | unused unless `TARGET_WOKWI 1` |
| 12 | **D9** / OC1A | Fan PWM (Timer1) | R3 → Q1 gate; R4 to +5V | `PIN_FAN_PWM 9` | `OUTPUT`, 4 kHz hardware PWM |
| 16 | D13 | Status LED (on-board) | — | `PIN_STATUS_LED 13` | `OUTPUT`, lit = fan ON |
| 23 | **A4** / SDA | I²C data | U3 **SDA**, R1 to +3V3 | fixed (hardware TWI) | internal pull-up **off** |
| 24 | **A5** / SCL | I²C clock | U3 **SCL**, R2 to +3V3 | fixed (hardware TWI) | internal pull-up **off** |
| 1, 2 | D1/TX, D0/RX | USB serial (programming, console) | — | — | leave free |
| 3, 28 | RESET | — | — | — | leave free |
| others | D10–D12, A0–A3, A6, A7, AREF | spare | — | — | — |

Why these pins (the firmware refuses to compile if they are changed in a way
that breaks this):

- **D9** is fixed: the hardware PWM comes from Timer1 output OC1A.
- **D2 / INT0** is the only external interrupt left after D3 is used by the
  encoder, and the tach needs a dedicated edge interrupt for accurate timing.
- **D3/D4** must both be on port D so a single pin-change interrupt (PCINT2)
  sees every edge of the encoder. D3's own INT1 is not used.
- The push switch and the keys are slow human inputs, so they are polled and
  debounced in `loop()`; they need no interrupt.

---

## 4. U3: AITRIP 1.3″ OLED + EC11 encoder module

Sold as SH1106, 128×64, I²C at address **0x3C**, supply **3.3 V**; EC11 with
20 pulses / 20 detents per turn, push switch on the shaft, plus two keys.
Evidence and checks: [VERIFICATION.md §5](VERIFICATION.md#5-hardware-assumptions-evidence-and-status).

> **Wire by the printed labels.** Listings for this module show different pin
> orders (for example `VCC GND SDA SCL TRA TRB PSH CON BAK` and
> `CON SDA SCL PSH TRA TRB BAK GND VCC`). Your board's silkscreen is the only
> authority.

| Module label | Function | Connects to |
|---|---|---|
| **VCC** | Supply | **+3V3** (U2 output) |
| **GND** | Ground | GND |
| **SDA** | I²C data | Nano A4, R1 2.2 kΩ to +3V3 |
| **SCL** | I²C clock | Nano A5, R2 2.2 kΩ to +3V3 |
| **TRA** | Encoder phase A | Nano D3 |
| **TRB** | Encoder phase B | Nano D4 |
| **PSH** | Encoder push button | Nano D5 |
| **CON** (KEY1) | "Confirm" key | Nano D6 (optional) |
| **BAK** (KEY0) | "Back" key | Nano D7 (optional) |

The encoder contacts and keys only ever pull to GND. The firmware enables the
ATmega's internal 5 V pull-ups on D3–D7; if the module has its own pull-ups
to 3.3 V the two share harmlessly (these pins do not connect to the SH1106).
The I²C lines are different: they are pulled up to 3.3 V only, and the
firmware turns the AVR's 5 V pull-ups off on A4/A5.

---

## 5. Discrete parts

### Semiconductors (pinouts as fitted)

| Ref | Part | Pins | Connections |
|---|---|---|---|
| **Q1** | 2N7000 N-MOSFET, TO-92 | flat face towards you, legs down: **S G D** | S → GND · G → R3 (to D9) and R4 (to +5V) · D → J1 pin 2 |
| **U2** | MCP1700-3302E/TO, TO-92 | flat face towards you, legs down: **GND VIN VOUT** | GND → GND · VIN → +5V, C3 · VOUT → +3V3, C4 |
| **D1** | 1N5817 Schottky, DO-41 | band = cathode | anode → F1 · cathode → +10V_P |
| **D2** | P6KE13A TVS, DO-15 | band = cathode | anode → GND · cathode → +10V_P |
| **D3** | BAT85 Schottky, DO-34 | band = cathode | anode → tach node (D2) · cathode → +5V |
| **D4** | BAT85 Schottky, DO-34 | band = cathode | anode → GND · cathode → tach node (D2) |

> Substitutes may have different pin orders. **BS170 is D-G-S** (reversed
> from the 2N7000). The SOT-23 MCP1700 has a different order. Check the
> datasheet of the part you actually fit.

### Resistors, capacitors, fuse

| Ref | Value | Between | Purpose |
|---|---|---|---|
| R1 | 2.2 kΩ | SDA ↔ +3V3 | I²C pull-up (3.3 V logic; low enough for 400 kHz rise times) |
| R2 | 2.2 kΩ | SCL ↔ +3V3 | I²C pull-up |
| R3 | 220 Ω | D9 ↔ Q1 gate | Limits D9's peak current into the gate capacitance; damps ringing |
| R4 | 10 kΩ | Q1 gate ↔ +5V | Holds Q1 on (fan **OFF**) while D9 floats during reset/bootloader |
| R5 | 4.7 kΩ **DNP** | J1 pin 2 ↔ +10V_P | Only if check M3 finds no internal pull-up |
| R6 | 10 kΩ | tach node (D2) ↔ +5V | Tach pull-up to **5 V** (never to 10 V) |
| R7 | 1 kΩ | J1 pin 1 ↔ tach node | Series protection: limits clamp current to ≤ 7 mA even with 12 V on pin 1 |
| C1 | 220 µF ≥ 25 V electrolytic | +10V_P ↔ GND | Bulk / hold-up (+ to the rail) |
| C2 | 100 nF ceramic | +10V_P ↔ GND | HF decoupling, close to Nano VIN |
| C3 | 1 µF ceramic | U2 VIN ↔ GND | LDO input capacitor |
| C4 | 1 µF ceramic | U2 VOUT ↔ GND | LDO output capacitor (stability) |
| C5 | 10 nF ceramic | tach node ↔ GND | Noise filter (τ ≈ 9 µs) |
| F1 | PTC 0.2 A hold / 0.4 A trip (Bourns MF-R020) | J1 pin 4 ↔ D1 anode | Protects the fan's supply if the controller shorts |

---

## 6. Point-to-point wiring list

For perfboard builds. Tick each line off as you solder it.

| # | From | To | Net |
|---|---|---|---|
| 1 | J1 pin 4 | F1 lead 1 | +10V_FAN |
| 2 | F1 lead 2 | D1 anode | |
| 3 | D1 cathode | Nano VIN (30), C1 +, C2, D2 cathode, R5* | **+10V_P** |
| 4 | J1 pin 3 | Nano GND (29), C1 −, C2, D2 anode, Q1 S, D4 anode, C5, C3, C4, U2 GND, U3 GND | **GND** |
| 5 | J1 pin 2 | Q1 drain, R5* | FAN_PWM |
| 6 | Nano D9 (12) | R3, R4 | PWM_GATE |
| 7 | R3 other end | Q1 gate | |
| 8 | R4 other end | +5V | |
| 9 | J1 pin 1 | R7 | TACH_IN |
| 10 | R7 other end | Nano D2 (5), R6, D3 anode, D4 cathode, C5 | TACH_5V |
| 11 | R6 other end, D3 cathode | +5V | |
| 12 | Nano +5V (27) | U2 VIN, C3, R4, R6, D3 cathode | **+5V** |
| 13 | U2 VOUT | U3 VCC, C4, R1, R2 | **+3V3** |
| 14 | Nano A4 (23) | U3 SDA, R1 other end | SDA |
| 15 | Nano A5 (24) | U3 SCL, R2 other end | SCL |
| 16 | Nano D3 (6) | U3 TRA | ENC_A |
| 17 | Nano D4 (7) | U3 TRB | ENC_B |
| 18 | Nano D5 (8) | U3 PSH | ENC_SW |
| 19 | Nano D6 (9) | U3 CON | KEY_CON (optional) |
| 20 | Nano D7 (10) | U3 BAK | KEY_BAK (optional) |

\* R5 only if check M3 says so.

**Layout tips:**

- Put C2 right at the Nano's VIN/GND pins.
- Keep Q1 and the J1 wiring away from the I²C wires.
- Use a socket (two 15-pin female headers) for the Nano, so you can test the
  board before the Nano is plugged in.
