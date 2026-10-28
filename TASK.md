# Task: Cloudline AI-CLS6 Fan Controller

Instructions for Claude to complete.

Using an AITRIP 1.3 inch OLED module with an integrated EC11 button/rotary encoder, and an Arduino Nano, design a controller for an AC Infinity Cloudline AI-CLS6 exhaust fan.  The AI-CLS6 uses a standard 4 pin Molex connection.

- The rotary encoder should control the fan speed
- The rotary encoder's button should toggle the fan on and off
- The fan must be off at power up
- The fan speed setting should persist as long as the device is powered
- The display should show both the fan setting and the fan speed based on feedback from the fan tachometer.  Display the on/off status, graphical bars, and numerical percentages.
- The Arduino and OLED module should be powered by the 10V supply from the fan. Include power conditioning if necessary.
- Provide a document detailing all pinouts including power supplies
- Provide a graphical schematic for all wiring including resistors and transistors
- Provide the Arduino code

ADDENDUM: Documentation, verification, tests, and completeness requirements

Treat everything below as mandatory. Do not mark the task complete until every item is delivered and the final checklist in section 9 is filled out.

1. README.md (primary entry point)
Write a README that takes someone with basic soldering skills and no Arduino experience from an empty bench to a working controller. It must include:
- What the device does and a photo-style overview diagram of the finished build.
- A complete bill of materials: part, quantity, exact value/rating, a specific purchasable part number or listing example, and approximate cost. Include connectors, wire gauge, heat shrink, and any enclosure.
- Required tools (soldering iron, multimeter, USB cable type, etc.).
- Software setup: Arduino IDE 2.x install, board selection, the exact library names and pinned versions, and the CH340 USB driver note for clone Nanos. State which processor setting to use ("ATmega328P" vs "ATmega328P (Old Bootloader)") and how to tell which one you need.
- A numbered step-by-step build guide in the order the work should actually be done: verify the fan connector, build power conditioning, test power with no Nano attached, wire the OLED/encoder, flash and test with USB power only, wire the fan interface, then first power-up on the fan supply. Each step ends with a "check before continuing" item that has a specific expected multimeter reading or on-screen result.
- First power-up procedure and what the display should show.
- Normal operation instructions.
- A troubleshooting table: symptom, likely cause, fix. Cover at minimum: blank display, garbled display, encoder skips or runs backwards, button not toggling, fan runs at full speed when it should be off, fan won't stop at 0%, RPM reads 0 or erratic, Nano resets when fan changes speed, upload fails.

2. FAQ.md
Answer the questions a builder will actually ask, including:
- Can I power the Nano from USB and the fan at the same time? (Address back-feeding and common ground.)
- Why does the fan stay off at power-up, and what happens during the Nano's reset/bootloader window?
- Why isn't the speed saved after power loss, and how would I add that if I wanted it?
- How is the RPM percentage calculated, and how do I calibrate max RPM for my fan?
- What PWM frequency is used and why? How do I change it?
- Will this work with other AC Infinity fans or other 4-pin fans?
- What happens if the fan is unplugged while running?
- Does this void the fan warranty or risk damaging the fan?
- Can I use an Uno, Pro Mini, or ESP32 instead?
- Is the 1.3" OLED SSD1306 or SH1106, and does it matter?

3. Hardware assumptions you must verify or flag
Do not present any of these as fact without a source. For each one, state the assumption, the evidence (datasheet, manufacturer doc, or reliable teardown with link), and a physical verification procedure the user performs with a multimeter before connecting the Nano:
- The AI-CLS6 connector pinout: which pin is supply, ground, PWM/control, and tach. If the connector is not a standard 4-pin PC fan layout, say so explicitly.
- The supply voltage and available current from the fan's connector, and whether it can safely power the Nano plus OLED.
- The control signal type: PWM (and at what voltage level and frequency) or 0-10V analog. If uncertain, design for the most likely case and document how to detect the other and what changes.
- Whether the control input has an internal pull-up and to what voltage, and what speed the fan runs at when the control line floats.
- Whether 0% command actually stops the fan or just runs it at minimum speed. If it doesn't stop, document the behavior and the options.
- Tach output type (open collector or push-pull), voltage level, and pulses per revolution.
- The OLED driver chip (SH1106 vs SSD1306), interface (I2C vs SPI), and I2C address for the AITRIP 1.3" module with integrated EC11.

Include a "pre-connection verification" section in the README with the exact measurements to take and the expected values. If a measurement doesn't match, the README must tell the user to stop and what to check.

4. Design issues that must be explicitly resolved in the code and schematic
- Interrupt allocation: the Nano has only two external interrupts (D2, D3). The encoder (A/B) and the tach together need more than two. Use pin change interrupts or a polled state machine where appropriate, and document the allocation and why.
- Boot-time fan state: during reset and the bootloader, the PWM pin is a floating input. If the fan runs at full speed when the control line floats, the "off at power up" requirement fails. Add a hardware pull resistor (or transistor biasing) that guarantees the off state before the sketch runs, and explain it.
- Level shifting: the Nano is 5V logic. Show how the PWM output is driven into a 10V-level input (open-collector NPN or N-MOSFET, with resistor values) and account for any resulting signal inversion in code. Show how the tach is pulled up to 5V, not 10V, and protected.
- Power conditioning: justify the choice (Nano VIN regulator vs a small buck module). Calculate regulator dissipation at 10V input with the OLED running. Include reverse-polarity protection, input bulk and decoupling capacitance, and transient protection appropriate for a motor supply. Address brownout resets when the motor changes speed.
- Encoder handling: robust debouncing or a quadrature state machine that doesn't skip or reverse. Define step size per detent and the speed range.
- Button handling: debounced, single toggle per press, no bounce-induced double toggles.
- RPM measurement: averaging window, timeout to report 0 RPM when pulses stop, and how the percentage is computed against a calibrated max RPM. Provide a calibration method (serial command or button hold at power-up) and document it.
- Display: no flicker, update rate stated, and the layout shows on/off status, setpoint bar and percentage, measured speed bar and percentage, and RPM.
- Persistence: setting kept in RAM only, as specified. Confirm the code does not write EEPROM.
- Add a watchdog timer so a hung sketch does not leave the fan in an unknown state, and document the reset behavior.
- Put every tunable value (pin numbers, PWM frequency, polarity, step size, pulses per rev, max RPM, minimum duty) in a clearly labeled config block at the top of the sketch.

5. Tests
- Separate the pure logic (encoder decoding, duty clamping and mapping, polarity inversion, RPM and percentage calculation, on/off state machine, power-up state) from hardware access so it can be unit tested on the host.
- Write host-side unit tests for that logic using PlatformIO's native environment or a minimal standalone C++ test harness. Include edge cases: encoder at min/max, rapid direction changes, button bounce, tach timeout, divide-by-zero guard, max RPM not yet calibrated.
- Run the tests and include the actual output.
- Compile the sketch for the Nano with arduino-cli (or PlatformIO) and include the compile output with flash and RAM usage. Confirm RAM headroom is adequate given the display buffer.
- If practical, provide a Wokwi simulation (diagram.json plus sketch) so the UI and encoder logic can be exercised before any hardware is bought. If the exact OLED or fan behavior can't be simulated, say what can and can't be.
- Hardware behavior can't be unit tested. Write BENCH_TEST.md: a step-by-step manual test procedure with expected results for every requirement, done first with a bench supply and a dummy load or LED in place of the fan, then with the real fan.
- If any of this is not appropriate or not possible, state why instead of skipping it silently.

6. Verification report (VERIFICATION.md)
After everything is built, perform a full independent review of all deliverables. Use a separate subagent that did not write the work to do the review, then reconcile its findings. The report must include:
- A requirements traceability matrix: every requirement from the original prompt and this addendum, mapped to the exact code location, schematic element, and test or bench step that satisfies it, with pass/fail status.
- Electrical calculations shown: resistor values, transistor saturation and base/gate drive, current draw budget, regulator dissipation, tach pull-up values, logic level thresholds.
- Cross-check that pin assignments are identical across the pinout document, schematic, code, and README. List every pin and confirm all four agree.
- A clear separation between what is verified (compiled, tested, calculated, sourced from a datasheet) and what is assumed and must be confirmed on hardware. Do not claim the design "will work" beyond what the evidence supports.
- Known limitations and failure modes, and what happens in each.
- Every issue found during review and how it was fixed.

7. Schematic and pinout quality
- The schematic must be a real rendered image (SVG and PNG), not ASCII only. Generate it from a source format (e.g., schemdraw, KiCad, or similar) and include the source so it can be edited.
- Label every component with reference designator and value. Show connector pin numbers as viewed from the mating side, and state which side is shown.
- Include a wiring diagram view in addition to the schematic, showing physical wire colors and connections for someone who doesn't read schematics.

8. Repository layout
Organize deliverables as:
- README.md, FAQ.md, BENCH_TEST.md, VERIFICATION.md, PINOUT.md
- /firmware (sketch, config, library list with versions)
- /test (unit tests and how to run them)
- /hardware (schematic source and renders, wiring diagram, BOM as CSV)
- /sim (Wokwi files, if provided)
- CHANGELOG.md

9. Final checklist
End your work by printing this checklist with each item marked done or not done, and a reason for any not done:
- All section 3 assumptions sourced or flagged with a verification procedure
- All section 4 design issues resolved and documented
- Unit tests written, run, passing, output included
- Sketch compiles for Nano, size report included
- Pin assignments consistent across all documents
- Independent review completed and findings resolved
- README walkthrough reviewed end to end for a first-time builder
