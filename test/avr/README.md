# AVR simulation test

`run_simavr.sh` compiles the real firmware for the Nano and runs the binary in
[simavr](https://github.com/buserror/simavr), an ATmega328P simulator. It
checks the serial output of two builds:

| Build | Extra flags | Checks |
|---|---|---|
| normal | `-DTEST_STATUS_EVERY_MS=1000` | boots, prints the banner, `loop()` keeps running, fan duty 0 %, no watchdog restart |
| hang | `-DTEST_STATUS_EVERY_MS=1000 -DTEST_HANG_AFTER_MS=3000` | after 3 s `loop()` stops feeding the watchdog; the watchdog interrupt restarts the sketch, which reports `restarted by the watchdog` and `reset=0x80` and starts with the fan OFF |

| hang-cli (information only) | `-DTEST_HANG_CLI_AFTER_MS=3000` | `loop()` stops with interrupts disabled. On the real chip the second timeout resets it (ATmega328P datasheet, WDTCSR bit WDIE). simavr does not model that rule, so the script only reports what happened. |

The `TEST_*` hooks at the end of `loop()` in `app.cpp` are compiled in only
when these flags are given.

Requirements: `arduino-cli` with `arduino:avr@1.8.6` and `U8g2@2.35.30`, and
`simavr` (Debian/Ubuntu: `apt install simavr`).

```sh
sh test/avr/run_simavr.sh
```

simavr has no SH1106, so the I2C transfers go nowhere; the display code
still runs. The status line also reports `ram_free_min`, the least free stack
seen since power-up (570 bytes in the recorded run).
