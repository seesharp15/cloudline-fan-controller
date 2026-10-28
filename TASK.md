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
