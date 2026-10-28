# Bill of materials

All parts are through-hole and easy to source, for perfboard assembly.
Reference designators match [schematic.svg](schematic.svg) and
[PINOUT.md](PINOUT.md).

| Ref | Qty | Part | Value / rating | Example part number | Substitutes and notes |
|---|---|---|---|---|---|
| U1 | 1 | Arduino Nano | ATmega328P, 5 V, 16 MHz | Arduino A000005 or any clone | CH340 clones are fine. Note which bootloader yours has (see the README). |
| U2 | 1 | 3.3 V LDO regulator | 250 mA, V<sub>in</sub> max 6 V, TO-92 | Microchip **MCP1700-3302E/TO** | HT7333-A (TO-92), or an AMS1117-3.3 breakout. **Check the pinout of your part.** |
| U3 | 1 | OLED + encoder module | 1.3″ SH1106 128×64 I²C, EC11 with push switch | AITRIP 1.3″ OLED Module EC11 | Wire by the printed labels |
| Q1 | 1 | N-channel MOSFET | 60 V, logic-level gate | **2N7000** (TO-92) | BS170 (**D-G-S pinout**, reversed). Any logic-level N-FET with V<sub>GS(th)</sub> < 2.5 V. |
| Q2 | 1 | NPN transistor | 40 V, 200 mA | **2N3904** (TO-92) | PN2222A (same E-B-C order), BC547 (**C-B-E**) |
| D1 | 1 | Schottky diode | 1 A, 20 V+ | **1N5817** | 1N5818, 1N5819, SB120 |
| D2 | 1 | TVS diode, unidirectional | V<sub>RWM</sub> 11.1 V, 600 W | **P6KE13A** | SMAJ12A (SMD). Use **P6KE15A** if the fan rail measures over 10.8 V. |
| F1 | 1 | PTC resettable fuse | 0.2 A hold / 0.4 A trip, ≥ 16 V | Bourns **MF-R020** | Littelfuse 30R020UU, or any 0.16–0.25 A hold radial PTC |
| C1 | 1 | Electrolytic capacitor | 100 µF, ≥ 25 V | Panasonic EEU-FR1E101 | 220 µF is fine too |
| C2 | 1 | Ceramic capacitor | 100 nF, 50 V | X7R, 5 mm pitch | |
| C3, C4 | 2 | Ceramic capacitor | 1 µF, ≥ 10 V | X7R | Required by the MCP1700 for stability |
| C5 | 1 | Ceramic capacitor | 1 nF, 50 V | C0G/X7R | |
| R1, R2 | 2 | Resistor | 4.7 kΩ, ¼ W | | |
| R3 | 1 | Resistor | 220 Ω, ¼ W | | 100–470 Ω all fine |
| R4, R6, R7, R8, R9 | 5 | Resistor | 10 kΩ, ¼ W | | |
| R5 | (1) | Resistor, **DNP** | 4.7 kΩ, ¼ W | | Fit only if VERIFICATION A2 says so |
| J1 | 1 | 4-pin Molex peripheral connector (5.08 mm), mating the fan's lead | | 4-pin Molex extension cable, 15–30 cm | Cut it and keep the end that plugs into the fan |
| — | 1 | 15-pin female headers ×2 (Nano socket) | 2.54 mm | | Lets you bench-test without the Nano |
| — | 1 | 9-pin female header or Dupont leads (module) | 2.54 mm | | |
| — | 1 | Perfboard, about 5 × 7 cm | | | |
| — | 1 | Enclosure | about 80 × 50 × 30 mm | | Cut-out for the OLED window and the encoder shaft. Knob for a 6 mm D shaft. |
| — | | Hook-up wire, heat-shrink, cable gland | | | |

**Spares worth having:**

- A second 2N7000 and 2N3904.
- A 4.7 kΩ resistor for R5.
- A P6KE15A, in case your fan's rail runs high.
