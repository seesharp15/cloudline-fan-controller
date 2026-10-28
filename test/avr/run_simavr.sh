#!/bin/sh
# Runs the real compiled Nano firmware in simavr (an ATmega328P instruction
# set simulator) and checks its serial output:
#   1. normal build: boots, prints the banner, loop() runs, fan OFF, the
#      watchdog is fed (no restart within the run);
#   2. hang build: loop() stops after 3 s (setting 37 %); the watchdog
#      interrupt must stop the fan and restart the sketch, which reports the
#      restart, comes back OFF, and keeps the 37 % setting.
#   3. hang-cli build (INFORMATION ONLY): loop() stops with interrupts
#      disabled, so the watchdog interrupt cannot run. On the real chip the
#      second timeout then resets it (ATmega328P datasheet, WDTCSR bit WDIE:
#      "If the interrupt is not executed before the next time-out, a System
#      Reset will be applied"). simavr does not model that rule: it keeps
#      raising the interrupt, so no reset is expected here.
#
# Needs arduino-cli with the arduino:avr core and U8g2, and simavr.
#   sh test/avr/run_simavr.sh            (from the repository root)
# ARDUINO_CLI may name a wrapper script instead of arduino-cli.
set -eu
CLI=${ARDUINO_CLI:-arduino-cli}
SKETCH=firmware/cloudline_controller
OUT=${TMPDIR:-/tmp}/cloudline-simavr
FQBN=arduino:avr:nano:cpu=atmega328
fail=0

build() { # name, extra flags
  $CLI compile -b $FQBN --build-path "$OUT/$1" \
    --build-property "compiler.cpp.extra_flags=$2" $SKETCH >"$OUT/$1-build.log" 2>&1 \
    || { echo "build '$1' failed, see $OUT/$1-build.log"; exit 1; }
}
run() { # name, seconds
  timeout "$2" simavr -m atmega328p -f 16000000 "$OUT/$1/cloudline_controller.ino.elf" \
    > "$OUT/$1.log" 2>&1 || true
}
expect() { # name, text
  if grep -q "$2" "$OUT/$1.log"; then echo "  ok   $1: $2"; else echo "  FAIL $1: missing '$2'"; fail=1; fi
}
reject() { # name, text
  if grep -q "$2" "$OUT/$1.log"; then echo "  FAIL $1: unexpected '$2'"; fail=1; else echo "  ok   $1: no '$2'"; fi
}

mkdir -p "$OUT"
echo "building..."
build normal "-DTEST_STATUS_EVERY_MS=1000 -DTEST_FORCE_DISPLAY"
build hang "-DTEST_STATUS_EVERY_MS=1000 -DTEST_FORCE_DISPLAY -DTEST_HANG_AFTER_MS=3000"
build hangcli "-DTEST_STATUS_EVERY_MS=1000 -DTEST_HANG_CLI_AFTER_MS=3000"
echo "running (simulated time is slower than real time)..."
run normal 20
run hang 30
run hangcli 30

expect normal "Cloudline controller ready. Fan OFF"
expect normal "on=0 set=50% duty=0.0%"
reject normal "restarted by the watchdog"
expect hang "TEST: loop() hangs now"
expect hang "restarted by the watchdog"
# After the restart the fan is OFF and the status shows the soft-restart flag.
expect hang "reset=0x80"
# The speed setting (37 % just before the hang) survives the restart; the fan is OFF.
expect hang "on=0 set=37% duty=0.0%"
expect hangcli "TEST: hang with interrupts off"
if grep -q "reset=0x8 " "$OUT/hangcli.log"; then
  echo "  info hangcli: simavr applied the watchdog reset"
else
  echo "  info hangcli: no reset (expected: simavr does not model WDIE-then-reset)"
fi
echo "logs: $OUT/normal.log $OUT/hang.log $OUT/hangcli.log"
[ $fail -eq 0 ] && echo "SIMAVR TESTS PASSED" || { echo "SIMAVR TESTS FAILED"; exit 1; }
