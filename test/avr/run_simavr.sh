#!/bin/sh
# Runs the real compiled Nano firmware in simavr (an ATmega328P instruction
# set simulator) and checks its serial output:
#   1. normal build: boots, prints the banner, loop() runs, fan OFF, the
#      watchdog is fed (no restart within the run);
#   2. hang build: loop() stops after 3 s; the watchdog interrupt must stop
#      the fan and restart the sketch, which then reports the restart.
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
    --build-property "compiler.cpp.extra_flags=$2" $SKETCH >/dev/null
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
build normal "-DTEST_STATUS_EVERY_MS=1000"
build hang "-DTEST_STATUS_EVERY_MS=1000 -DTEST_HANG_AFTER_MS=3000"
echo "running (simulated time is slower than real time)..."
run normal 20
run hang 30

expect normal "Cloudline controller ready. Fan OFF"
expect normal "on=0 set=50% duty=0.0%"
reject normal "restarted by the watchdog"
expect hang "TEST: loop() hangs now"
expect hang "restarted by the watchdog"
# After the restart the fan is OFF and the status shows the soft-restart flag.
expect hang "reset=0x8"
echo "logs: $OUT/normal.log $OUT/hang.log"
[ $fail -eq 0 ] && echo "SIMAVR TESTS PASSED" || { echo "SIMAVR TESTS FAILED"; exit 1; }
