# FAQ

Questions builders ask, with the reasoning behind the design. Calculations
are in [VERIFICATION.md §2](VERIFICATION.md#2-electrical-calculations); part
connections in [PINOUT.md](PINOUT.md).

- [Can I power the Nano from USB and the fan at the same time?](#can-i-power-the-nano-from-usb-and-the-fan-at-the-same-time)
- [Why does the fan stay off at power-up, and what happens during the reset/bootloader window?](#why-does-the-fan-stay-off-at-power-up-and-what-happens-during-the-resetbootloader-window)
- [Why isn't the speed saved after power loss, and how would I add that?](#why-isnt-the-speed-saved-after-power-loss-and-how-would-i-add-that)
- [How is the RPM percentage calculated, and how do I calibrate max RPM?](#how-is-the-rpm-percentage-calculated-and-how-do-i-calibrate-max-rpm)
- [What PWM frequency is used and why? How do I change it?](#what-pwm-frequency-is-used-and-why-how-do-i-change-it)
- [Will this work with other AC Infinity fans or other 4-pin fans?](#will-this-work-with-other-ac-infinity-fans-or-other-4-pin-fans)
- [What happens if the fan is unplugged while running?](#what-happens-if-the-fan-is-unplugged-while-running)
- [Does this void the fan warranty or risk damaging the fan?](#does-this-void-the-fan-warranty-or-risk-damaging-the-fan)
- [Can I use an Uno, Pro Mini, or ESP32 instead?](#can-i-use-an-uno-pro-mini-or-esp32-instead)
- [Is the 1.3" OLED SSD1306 or SH1106, and does it matter?](#is-the-13-oled-ssd1306-or-sh1106-and-does-it-matter)
- [Does 0% or OFF actually stop the fan?](#does-0-or-off-actually-stop-the-fan)
- [What does the watchdog do, and is it safe with the old bootloader?](#what-does-the-watchdog-do-and-is-it-safe-with-the-old-bootloader)
- [Why does the tach input pull up to 5 V and not to the fan's 10 V?](#why-does-the-tach-input-pull-up-to-5-v-and-not-to-the-fans-10-v)
- [Why the Nano's own regulator and not a buck converter?](#why-the-nanos-own-regulator-and-not-a-buck-converter)
- [What if the fan's port can't supply enough current?](#what-if-the-fans-port-cant-supply-enough-current)
- [Why doesn't the knob toggle the fan the moment I press it?](#why-doesnt-the-knob-toggle-the-fan-the-moment-i-press-it)

---

## Can I power the Nano from USB and the fan at the same time?

**Yes.** On the Nano, USB 5 V reaches the board's 5 V rail through a Schottky
diode, and the VIN regulator's 5 V output feeds the same rail directly. With
both present, the regulator's 5.0 V is higher than USB minus the diode
(about 4.7 V), so the board runs from the regulator and the diode stops any
current flowing back into the computer.

- **Back-feeding into the fan:** with USB connected and the fan unpowered,
  5 V can leak backwards through the Nano's regulator to VIN (many
  regulators do this). D1 blocks it from reaching the fan's +10 V pin. The
  fan's speed line only ever sees Q1 (which can only pull it low), and the
  tach pin only sees R7 (1 kΩ) and the 5 V pull-up, so nothing drives the
  fan's pins.
- **Common ground:** the computer's USB ground is connected to the
  controller's ground, which is the fan's low-voltage ground (J1 pin 3).
  That is normal and safe as long as the fan's control port is isolated from
  mains, which check M6 confirms. On a desktop PC, USB ground is usually tied
  to mains earth; this only joins two grounds.
- **During an upload** the Nano resets, so the fan stops (R4 holds it off)
  and the controller starts again OFF.

## Why does the fan stay off at power-up, and what happens during the reset/bootloader window?

The fan's speed line is pulled up inside the fan, and builders report that a
released (high) line means full speed. So any moment in which nothing pulls
the line low would make the fan run flat out. The controller makes sure
something always does:

| Time after power-up | D9 (Nano pin) | Q1 gate | Fan speed line | Fan |
|---|---|---|---|---|
| 0 to ~1 ms: 5 V rail rising | floating | follows +5V through R4; below Q1's threshold for a fraction of a millisecond | briefly released | no reaction: far too short, and the fan's own electronics are starting too |
| reset (~65 ms) and bootloader (up to ~2 s on old-bootloader clones) | **floating input** | **held at 5 V by R4** | **held low by Q1** | **stopped** |
| `setup()` starts | written HIGH, *then* made an output | 5 V | low | stopped |
| `setup()` done | firmware state: OFF | 5 V | low | stopped, until you press the knob |

R4 (10 kΩ from the Q1 gate to +5V) is the key part: a MOSFET draws no steady
gate current, so the resistor alone holds Q1 on. The firmware then drives D9
HIGH *before* switching it to an output, so there's no glitch at hand-over
(the simulation checks this). The same applies after a watchdog restart, an
upload, or a brown-out reset.

If check M4 shows your fan runs the other way round (line low = full speed),
R4 must go to GND instead and `FAN_PWM_INVERTED` set to 0; otherwise the fan
would run at boot.

## Why isn't the speed saved after power loss, and how would I add that?

Because the brief asks for the setting to persist *as long as the device is
powered*, and for the fan to be OFF at power-up. The firmware keeps the
setting only in RAM and writes nothing to EEPROM; the test suite fails if
any EEPROM access appears in the firmware.

To save the **setting** (not the on/off state, so the fan still starts OFF):

1. In `app.cpp`, `#include <EEPROM.h>`.
2. In `setup()`, after `gCtl.begin(...)`: read a byte with
   `EEPROM.read(0)`; if it is 0–100, call `gCtl.setSetting(value)`.
3. In `loop()`, remember when the setting last changed, and once it has been
   unchanged for 10 s and differs from the stored value, call
   `EEPROM.update(0, gCtl.setting())`.

Waiting 10 s means one write per adjustment rather than one per click. The
ATmega328P's EEPROM is rated for 100 000 writes per byte, which at that rate
is decades of normal use. Remove the `no-eeprom` check from `test/Makefile`
if you do this.

## How is the RPM percentage calculated, and how do I calibrate max RPM?

1. The tach interrupt counts falling edges on D2 and records the time of
   the last one (`micros()`). Edges closer than 1 ms are ignored as noise.
2. Every 500 ms (`TACH_WINDOW_MS`) the main loop computes the frequency
   from the **time between the last edge of the previous window and the last
   edge of this one**, divided into the number of edges in between. Measuring
   the period like this stays accurate at low speed, where a 500 ms window
   holds only a few pulses.
3. RPM = frequency × 60 ÷ `TACH_PULSES_PER_REV` (default 2).
4. No edge for 1.5 s (`TACH_TIMEOUT_MS`) → 0 RPM.
5. The displayed RPM is smoothed (each update moves ¼ of the way to the new
   value); it jumps straight to 0 or to the first reading when the fan stops
   or starts.
6. **ACT %** = RPM × 100 ÷ max RPM, rounded, capped at 100. If max RPM is
   unknown (0), the display shows `--%` instead of dividing by zero.

AC Infinity publishes no RPM figure for the AI-CLS6, and the tach's pulses
per revolution are unknown, so **max RPM is measured, not assumed**:

- **Calibrate:** with the fan ON, hold the knob for 3 s (or send `c` on the
  serial console). The fan runs at 100 % for 12 s to spin up, then the RPM is
  averaged for 6 s. The result is accepted only if it is at least 200 RPM
  and every sample is within ±10 % of the average (`CAL FAILED` otherwise,
  e.g. no tach or gusty ducting). Afterwards the fan returns to your setting.
- The result lasts until power-off. To keep it, copy the
  `#define TACH_MAX_RPM ...` line the controller prints on the serial console
  into `config.h` and upload. You can also set it for one session with
  `m 1836`.
- Because both the measurement and the calibration use the same
  pulses-per-revolution value, **ACT % is correct even if
  `TACH_PULSES_PER_REV` is wrong**; only the RPM figure would be off by that
  factor. To check the RPM figure itself you need an independent reference,
  such as an optical (laser) tachometer and reflective tape on the impeller.

## What PWM frequency is used and why? How do I change it?

**4 kHz.** A builder measured the stock AC Infinity controller's signal on a
Cloudline S6 at a period of about 245 µs (≈ 4 kHz), and other builders report
1 kHz and about 5 kHz also working. Matching the stock controller is the
safest choice.

The PWM comes from Timer1 in fast-PWM mode 14 with no prescaler:
TOP = 16 MHz ÷ 4 kHz − 1 = 3999, so the duty has 4000 steps (0.025 %),
far finer than the 1 % knob steps. 0 % and 100 % are not PWM at all: the pin
is driven as a constant, which avoids the one-tick spike fast PWM produces at
the extremes.

To change it, set `FAN_PWM_FREQ_HZ` in `config.h` (250 to 20 000 Hz; the
build fails outside that range). Keep in mind the fan's internal pull-up and
the cable capacitance set how quickly the line rises: with a 100 kΩ pull-up
and 300 pF of cable the rise time constant is 30 µs, which is 12 % of a 4 kHz
period but 60 % of a 20 kHz one. Lower frequencies are more forgiving.

## Will this work with other AC Infinity fans or other 4-pin fans?

- **Other Cloudline EC fans with the same 4-pin Molex control port**
  (S4, S6, S8, T4, T6, T8, T10, T12): builders report the same port and pin
  functions on several of these, so very likely yes. Do the pre-connection
  verification on each fan. Their RPM and stop behaviour differ, so
  calibrate.
- **Newer models with a USB-C ("UIS") control port** (some later S6 units
  and the Cloudline PRO / EVO series): **not plug-compatible.** One builder
  documents the USB-C port as VBUS = +10 V, D+ = tach, D− = PWM. The same
  circuit idea may well work through a suitable breakout, but this has not
  been verified; check M1–M5 still apply.
- **PC 4-pin fans** (Intel 4-wire spec): electrically similar
  (open-drain-friendly PWM input with a pull-up, open-collector tach with 2
  pulses per revolution), but they need a **12 V supply** you provide, a
  different connector, and PWM at **21–28 kHz**. You would power the
  controller from that 12 V (use a P6KE15A for D2), wire the fan's pins to
  J1's functions, raise the frequency limit in `app.cpp` and set
  `FAN_PWM_FREQ_HZ 25000`. Not tested.
- **3-pin or 2-pin fans** have no PWM input: not supported.

## What happens if the fan is unplugged while running?

The controller is powered by the fan, so:

- **Fan unplugged from mains, or J1 pulled out:** the controller loses power
  and the display goes dark. When power returns, it starts **OFF at 50 %**,
  as the brief requires. **The fan will not resume on its own after a power
  cut.** If you need ventilation to restart automatically, this behaviour
  must be changed deliberately.
- **Only the tach wire disconnected:** the fan keeps running; after 8 s the
  display shows `! NO TACH / STALLED` and ACT `--%`.
- **Only the speed wire disconnected:** the fan's line is then released, and
  builder reports suggest it runs at full speed; the controller can't
  prevent that. The display still shows your setting, and the measured
  speed (if the tach works) shows the difference.
- **Controller's 5 V fails while the fan has power** (for example the Nano
  dies): R4 has no supply, Q1 turns off, the line is released, and the fan
  most likely runs at full speed. For an exhaust fan, "ventilate if the
  controller dies" is usually the safer failure.

## Does this void the fan warranty or risk damaging the fan?

**Warranty:** AC Infinity's warranty terms were not reviewed for this
project, so this can't be answered with authority. The controller does not
open or modify the fan; it plugs into the control port that AC Infinity's
own controllers use, through an ordinary extension cable. Ask AC Infinity if
it matters to you.

**Damage risk:** low by design, and checked by the pre-connection
measurements:

- Nothing in the controller drives a voltage into the fan. Q1 can only pull
  the speed line towards ground, which is what the stock controller does too
  (it "grounds and opens" the line, per a builder's measurements). The tach
  pin sees only a 1 kΩ resistor to a clamped 5 V node.
- The controller draws about 35–60 mA from the fan's +10 V, and check M2
  confirms the port holds its voltage at 45 mA. F1 limits the current to
  well under half an amp if the controller develops a short.
- Plugging the controller in the wrong way round is prevented by the
  connector, and D1 protects against a mis-wired cable.

## Can I use an Uno, Pro Mini, or ESP32 instead?

| Board | Works? | Notes |
|---|---|---|
| **Arduino Uno** (R3, ATmega328P) | **Yes, unchanged** | Same chip, same pin numbers, same firmware. Select *Arduino Uno*. VIN accepts the fan's ~9.6 V. Larger. |
| **Pro Mini 5 V / 16 MHz** | **Yes, with care** | Same chip and firmware. Its small regulator would dissipate about 0.25 W from 9.6 V, which is too hot for its tiny package: feed **5 V** to its VCC from a separate regulator (e.g. an LM7805 or a small buck module) instead of using RAW. Needs a USB-serial adapter for uploads and the console. |
| **Pro Mini 3.3 V / 8 MHz** | Not recommended | 3.3 V logic can't reliably switch a 2N7000 (its threshold can be up to 3 V). Use a logic-level MOSFET rated at V<sub>GS</sub> = 2.5 V, move R4 and R6 to 3.3 V, and expect to re-verify everything. |
| **ESP32** | **No** (firmware not portable as-is) | Different timers and interrupts, 3.3 V logic that is **not 5 V tolerant** (R6 and D3 would go to 3.3 V), and Wi-Fi current peaks the fan's port may not supply. Many ESPHome builds control Cloudline fans this way; see the sources in the README. |

## Is the 1.3" OLED SSD1306 or SH1106, and does it matter?

The AITRIP module and its equivalents are sold as **SH1106**, I²C, address
0x3C. Most 1.3″ 128×64 OLEDs use the SH1106; the smaller 0.96″ ones use the
SSD1306. A few listings for similar modules say SSD1306 or SH1116, so check
yours.

It matters because the two chips are commanded differently: the SH1106 has
132 columns of memory for a 128-column panel, so SSD1306 code shows the
picture shifted by 2 pixels with a stripe of garbage at one edge, and SH1106
code on an SSD1306 shows a similarly corrupted picture. If the display is
garbled, set `OLED_SSD1306 1` in `config.h` and upload. If it's blank, try
`OLED_I2C_ADDRESS 0x3D`. Both are I²C; a 7-pin SPI version of the 1.3″ OLED
exists but is a different module.

## Does 0% or OFF actually stop the fan?

**Expected yes, confirmed per fan by check M4.** OFF and a 0 % setting both
hold the speed line at 0 V continuously. A builder's measurements of the
stock controller show its "speed 0" is exactly that (0 µs high time), and
another builder saw 0 tach pulses at speed 0. If M4 shows your fan only
slows to a minimum speed with the line held low, then:

- OFF means "minimum speed" on that fan; the controller can't remove the
  fan's mains power because it is powered by the fan.
- To stop it completely, switch the fan's mains supply (a switched outlet or
  smart plug); the controller will come back OFF when power returns.

Separately, many EC fans won't **start** below a certain duty (the stock
controller's lowest speed is about 33 % duty). If the fan doesn't turn at
low settings, raise `FAN_DUTY_MIN_PCT` so that 1 % maps to the lowest duty
that turns it ([BENCH_TEST.md B6](BENCH_TEST.md#part-b-the-real-fan)).

## What does the watchdog do, and is it safe with the old bootloader?

If `loop()` ever stops running for 2 s (`WATCHDOG_TIMEOUT`), the ATmega's
watchdog fires. The usual "reset the chip" approach has a known problem: on
Nanos with the **old bootloader** (the ones that need *Processor: ATmega328P
(Old Bootloader)*), a watchdog reset leaves the watchdog running with a
16 ms timeout, the bootloader doesn't turn it off, and the board resets
endlessly until it is power-cycled. Optiboot ("ATmega328P") doesn't have
this bug.

This firmware avoids the problem by using the watchdog's **interrupt mode
first**:

1. First timeout: the watchdog interrupt runs. It immediately switches Timer1
   off D9 and drives D9 HIGH, so **Q1 pulls the line low and the fan stops**,
   then restarts the sketch at address 0. That skips the bootloader, so the
   old-bootloader bug can't happen. `setup()` starts again with the fan OFF,
   exactly like a power-up, and prints `! restarted by the watchdog`.
2. If even the interrupt can't run (interrupts disabled by a deeper fault),
   the second timeout does a real reset. The ATmega328P datasheet (WDTCSR,
   bit WDIE) says: "If the interrupt is not executed before the next
   time-out, a System Reset will be applied." On an Optiboot Nano that is a
   normal restart. On an old-bootloader Nano it may loop in the bootloader,
   but D9 floats during that loop, so **R4 keeps the fan stopped** until you
   power-cycle.

Either way the fan ends up in a known state: stopped. Case 1 has been
exercised on the real compiled firmware in an AVR simulator
([test/avr](test/avr/README.md)); case 2 rests on the datasheet, because the
simulator does not model that rule (BENCH_TEST.md W2 tests it on hardware).
Set `WATCHDOG_ENABLE 0` to turn the watchdog off.

## Why does the tach input pull up to 5 V and not to the fan's 10 V?

So that the Nano's pin can never see more than 5 V, whatever the tach pin
turns out to be. Nothing guarantees what is on J1 pin 1: possibly an
open-collector tach, possibly a push-pull output, possibly nothing.

- **R6** (10 kΩ to +5V) supplies the "high" level for an open-collector
  output.
- **R7** (1 kΩ) sits between the fan and everything else. When the fan pulls
  low, D2 sees 5 V × 1k/11k ≈ **0.45 V**, well below the ATmega's 1.5 V
  "low" threshold.
- If the pin is driven high by the fan to as much as 12 V, **D3** clamps the
  node to about 5.3 V and R7 limits that current to (12 − 5.3) / 1 kΩ ≈
  **6.7 mA**, harmless to the fan and the Schottky. **D4** does the same for
  negative spikes.
- **C5** (10 nF) with the ~0.9 kΩ source resistance filters spikes shorter
  than about 10 µs. Tach pulses are milliseconds long.

## Why the Nano's own regulator and not a buck converter?

At this current the linear regulator is fine and quieter. From ~9.6 V at the
VIN pin to 5 V at up to 60 mA it dissipates about 0.28 W, which raises its
temperature by roughly 25 °C on the Nano's small copper area: comfortably
within limits even in a warm grow tent. A buck module would save about
0.2 W but adds switching noise next to the tach input and another part to
fail. Above roughly 100 mA, or with a supply above 12 V, a buck module would
be the better choice. Details in VERIFICATION.md §2.

## What if the fan's port can't supply enough current?

If check M2 shows the voltage sagging by more than 1 V under 45 mA, power the
controller from a separate 9–12 V DC adapter: connect its + to F1 (instead of
J1 pin 4) and its − to GND, keep J1 pin 3 (GND) connected so the speed and
tach signals share a ground with the fan, and leave J1 pin 4 unconnected.
Use a P6KE15A for D2 with a 12 V adapter.

## Why doesn't the knob toggle the fan the moment I press it?

The same button also starts a calibration when held for 3 s, so the firmware
waits for the release to know which you meant. A short press toggles on
release (after a 25 ms debounce); a hold never toggles.
