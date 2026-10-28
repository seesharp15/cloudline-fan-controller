// Unit tests for firmware/cloudline_controller/fan_logic.h.
#include "../firmware/cloudline_controller/fan_logic.h"

#include "check.h"

using namespace fan;

// Feed a sequence of (A,B) levels and return the summed detents.
static int feed(QuadratureDecoder &d, const int (*seq)[2], int n) {
  int sum = 0;
  for (int i = 0; i < n; i++) sum += d.update(seq[i][0], seq[i][1]);
  return sum;
}

TEST(decoder_counts_one_detent_per_full_cycle) {
  QuadratureDecoder d;
  const int cw[][2] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
  const int ccw[][2] = {{1, 0}, {0, 0}, {0, 1}, {1, 1}};
  CHECK_EQ(feed(d, cw, 4), 1);
  CHECK_EQ(feed(d, cw, 4), 1);
  CHECK_EQ(feed(d, ccw, 4), -1);
}

TEST(decoder_emits_only_at_rest_position) {
  QuadratureDecoder d;
  CHECK_EQ(d.update(0, 1), 0);
  CHECK_EQ(d.update(0, 0), 0);
  CHECK_EQ(d.update(1, 0), 0);
  CHECK_EQ(d.update(1, 1), 1);
}

TEST(decoder_ignores_bounce_and_aborted_turns) {
  QuadratureDecoder d;
  // Contact chatter on the first edge, then the rest of the detent.
  const int bouncy[][2] = {{0, 1}, {1, 1}, {0, 1}, {1, 1}, {0, 1},
                           {0, 0}, {0, 1}, {0, 0}, {1, 0}, {1, 1}};
  CHECK_EQ(feed(d, bouncy, 10), 1);
  // Half a detent forwards, then back to rest: no step.
  const int aborted[][2] = {{0, 1}, {0, 0}, {0, 1}, {1, 1}};
  CHECK_EQ(feed(d, aborted, 4), 0);
  // Repeated level (no edge) is ignored.
  CHECK_EQ(d.update(1, 1), 0);
}

TEST(decoder_discards_invalid_double_jump) {
  QuadratureDecoder d;
  // 11 -> 01 -> 00 -> (10 missed) -> 11: the 00 -> 11 jump has no direction,
  // so the detent is dropped instead of guessed.
  const int skipped[][2] = {{0, 1}, {0, 0}, {1, 1}};
  CHECK_EQ(feed(d, skipped, 3), 0);
  // The decoder is back in sync for the next clean detent.
  const int cw[][2] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
  CHECK_EQ(feed(d, cw, 4), 1);
}

TEST(decoder_reset_resynchronises) {
  QuadratureDecoder d;
  d.reset(0, 0);  // e.g. powered up between detents
  CHECK_EQ(d.update(1, 0), 0);
  CHECK_EQ(d.update(1, 1), 0);  // only 2 edges since reset: not a detent
  const int cw[][2] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
  CHECK_EQ(feed(d, cw, 4), 1);
}

TEST(button_short_press_after_debounce) {
  Button b(25, 3000);
  CHECK(b.update(true, 0) == ButtonEvent::None);
  CHECK(b.update(true, 10) == ButtonEvent::None);
  CHECK(!b.held());
  CHECK(b.update(true, 26) == ButtonEvent::None);
  CHECK(b.held());
  CHECK(b.update(false, 200) == ButtonEvent::None);  // release not yet stable
  CHECK(b.update(false, 226) == ButtonEvent::Short);
  CHECK(!b.held());
}

TEST(button_ignores_glitches) {
  Button b(25, 3000);
  for (uint32_t t = 0; t < 200; t += 10) {
    // 5 ms spikes never last long enough to count
    CHECK(b.update(true, t) == ButtonEvent::None);
    CHECK(b.update(false, t + 5) == ButtonEvent::None);
  }
  CHECK(!b.held());
}

TEST(button_long_press_fires_once_and_suppresses_short) {
  Button b(25, 3000);
  b.update(true, 0);
  b.update(true, 25);
  CHECK(b.update(true, 3000) == ButtonEvent::None);  // held 2975 ms
  CHECK(b.update(true, 3025) == ButtonEvent::Long);
  CHECK(b.update(true, 5000) == ButtonEvent::None);
  b.update(false, 6000);
  CHECK(b.update(false, 6025) == ButtonEvent::None);
  CHECK_EQ(b.heldMs(6025), 0);
}

TEST(button_suppressed_press_produces_nothing) {
  Button b(25, 3000);
  b.update(true, 0);
  b.suppressUntilRelease();
  b.update(true, 30);
  CHECK(b.update(true, 5000) == ButtonEvent::None);
  b.update(false, 5100);
  CHECK(b.update(false, 5130) == ButtonEvent::None);
  // The next press works normally.
  b.update(true, 6000);
  b.update(true, 6030);
  b.update(false, 6100);
  CHECK(b.update(false, 6130) == ButtonEvent::Short);
}

TEST(button_bounce_gives_one_press) {
  // A real press: 8 ms of chatter, held 150 ms, 6 ms of release chatter.
  Button b(25, 3000);
  int shorts = 0, longs = 0;
  uint32_t t = 0;
  auto step = [&](bool level) {
    const ButtonEvent e = b.update(level, t++);
    shorts += e == ButtonEvent::Short;
    longs += e == ButtonEvent::Long;
  };
  for (int i = 0; i < 8; i++) step(i % 2 == 0);
  for (int i = 0; i < 150; i++) step(true);
  for (int i = 0; i < 6; i++) step(i % 2 == 1);
  for (int i = 0; i < 100; i++) step(false);
  CHECK_EQ(shorts, 1);
  CHECK_EQ(longs, 0);
  // Ten such presses: ten events, never more.
  for (int p = 0; p < 10; p++) {
    for (int i = 0; i < 8; i++) step(i % 2 == 0);
    for (int i = 0; i < 80; i++) step(true);
    for (int i = 0; i < 6; i++) step(i % 2 == 1);
    for (int i = 0; i < 80; i++) step(false);
  }
  CHECK_EQ(shorts, 11);
}

TEST(detents_step_accelerate_and_clamp) {
  CHECK_EQ(applyDetents(50, 1, 100, 40, 1, 5, 0, 100), 51);
  CHECK_EQ(applyDetents(50, -2, 100, 40, 1, 5, 0, 100), 48);
  CHECK_EQ(applyDetents(50, 1, 10, 40, 1, 5, 0, 100), 55);
  CHECK_EQ(applyDetents(98, 1, 10, 40, 1, 5, 0, 100), 100);
  CHECK_EQ(applyDetents(2, -3, 10, 40, 1, 5, 0, 100), 0);
  CHECK_EQ(applyDetents(100, 100, 100, 40, 1, 5, 0, 100), 100);
  CHECK_EQ(applyDetents(0, -100, 100, 40, 1, 5, 0, 100), 0);
  CHECK_EQ(applyDetents(42, 0, 0, 40, 1, 5, 0, 100), 42);
}

TEST(setting_to_duty_mapping) {
  CHECK_EQ(settingToDutyPermille(50, false, 0, 100), 0);  // OFF always 0
  CHECK_EQ(settingToDutyPermille(0, true, 0, 100), 0);
  CHECK_EQ(settingToDutyPermille(1, true, 0, 100), 10);
  CHECK_EQ(settingToDutyPermille(50, true, 0, 100), 500);
  CHECK_EQ(settingToDutyPermille(100, true, 0, 100), 1000);
  CHECK_EQ(settingToDutyPermille(150, true, 0, 100), 1000);
  // With a start threshold: 1 % is just above the minimum duty.
  CHECK_EQ(settingToDutyPermille(1, true, 20, 100), 208);
  CHECK_EQ(settingToDutyPermille(100, true, 20, 100), 1000);
  CHECK_EQ(settingToDutyPermille(100, true, 20, 90), 900);
  // Monotonic over the whole range.
  uint16_t prev = 0;
  for (int s = 1; s <= 100; s++) {
    const uint16_t d = settingToDutyPermille((uint8_t)s, true, 20, 100);
    CHECK(d > prev);
    prev = d;
  }
}

TEST(gate_drive_inverted) {
  const uint16_t top = 15999;  // 1 kHz at 16 MHz
  GateDrive g = gateDriveFor(0, top, true);  // fan stopped: gate HIGH constantly
  CHECK(g.mode == GateDrive::ConstantHigh);
  g = gateDriveFor(1000, top, true);  // full speed: gate LOW constantly
  CHECK(g.mode == GateDrive::ConstantLow);
  g = gateDriveFor(500, top, true);
  CHECK(g.mode == GateDrive::Pwm);
  CHECK_EQ(g.compare + 1, 8000);  // HIGH for half of 16000 ticks
  g = gateDriveFor(250, top, true);  // fan line high 25 % -> gate high 75 %
  CHECK_EQ(g.compare + 1, 12000);
  g = gateDriveFor(1, top, true);  // 0.1 % fan duty still leaves a LOW tick
  CHECK(g.mode == GateDrive::Pwm);
  CHECK(g.compare < top);
  g = gateDriveFor(2000, top, true);  // out of range clamps to full speed
  CHECK(g.mode == GateDrive::ConstantLow);
}

TEST(gate_drive_non_inverted) {
  GateDrive g = gateDriveFor(0, 799, false);
  CHECK(g.mode == GateDrive::ConstantLow);
  g = gateDriveFor(1000, 799, false);
  CHECK(g.mode == GateDrive::ConstantHigh);
  g = gateDriveFor(300, 799, false);
  CHECK(g.mode == GateDrive::Pwm);
  CHECK_EQ(g.compare + 1, 240);
}

TEST(timer1_top) {
  CHECK_EQ(timer1TopFor(16000000UL, 1, 1000), 15999);
  CHECK_EQ(timer1TopFor(16000000UL, 1, 5000), 3199);
  CHECK_EQ(timer1TopFor(16000000UL, 1, 20000), 799);
  CHECK_EQ(timer1TopFor(16000000UL, 1, 250), 63999);
  CHECK_EQ(timer1TopFor(16000000UL, 1, 100), 65535);  // clamped
}

// Drive the estimator with an ideal pulse train sampled every windowMs.
static uint16_t runTach(TachEstimator &t, double hz, uint32_t windowMs, int windows,
                        uint32_t &count, uint32_t &lastUs, double &nextUs, uint64_t &nowUs) {
  uint16_t rpm = 0;
  for (int w = 0; w < windows; w++) {
    const uint64_t end = nowUs + windowMs * 1000ull;
    if (hz > 0) {
      if (nextUs < (double)nowUs) nextUs = (double)nowUs + 1e6 / hz;
      while (nextUs <= (double)end) {
        count++;
        lastUs = (uint32_t)nextUs;
        nextUs += 1e6 / hz;
      }
    }
    nowUs = end;
    rpm = t.update(count, lastUs, (uint32_t)nowUs);
  }
  return rpm;
}

TEST(tach_estimates_rpm_from_periods) {
  TachEstimator t(2, 1500000);
  uint32_t count = 0, lastUs = 0;
  double next = 0;
  uint64_t now = 0;
  CHECK_EQ(runTach(t, 0, 500, 3, count, lastUs, next, now), 0);
  // 50 Hz at 2 pulses/rev = 1500 RPM. First window only sets the reference.
  CHECK_EQ(runTach(t, 50, 500, 1, count, lastUs, next, now), 0);
  CHECK_NEAR(runTach(t, 50, 500, 1, count, lastUs, next, now), 1500, 2);
  CHECK_NEAR(t.frequencyMilliHz(), 50000, 50);
  // Odd frequency that does not divide the window: still exact.
  CHECK_NEAR(runTach(t, 33.3, 500, 4, count, lastUs, next, now), 999, 2);
  // Slow fan: 1.5 Hz (45 RPM) gives < 1 pulse per window but is still measured.
  CHECK_NEAR(runTach(t, 1.5, 500, 12, count, lastUs, next, now), 45, 1);
}

TEST(tach_reports_zero_after_timeout) {
  TachEstimator t(2, 1500000);
  uint32_t count = 0, lastUs = 0;
  double next = 0;
  uint64_t now = 0;
  runTach(t, 40, 500, 4, count, lastUs, next, now);
  CHECK_NEAR(t.rpm(), 1200, 2);
  // Stops: the last value is held until the 1.5 s timeout, then 0.
  CHECK_NEAR(runTach(t, 0, 500, 2, count, lastUs, next, now), 1200, 2);
  CHECK_EQ(runTach(t, 0, 500, 2, count, lastUs, next, now), 0);
  CHECK_EQ(t.frequencyMilliHz(), 0);
  // Restarting re-establishes the reference before reporting again.
  next = 0;
  runTach(t, 40, 500, 3, count, lastUs, next, now);
  CHECK_NEAR(t.rpm(), 1200, 2);
}

TEST(tach_pulses_per_rev) {
  TachEstimator t1(1, 1500000), t4(4, 1500000);
  uint32_t c1 = 0, l1 = 0, c4 = 0, l4 = 0;
  double n1 = 0, n4 = 0;
  uint64_t now1 = 0, now4 = 0;
  CHECK_NEAR(runTach(t1, 20, 500, 3, c1, l1, n1, now1), 1200, 2);
  CHECK_NEAR(runTach(t4, 20, 500, 3, c4, l4, n4, now4), 300, 1);
}

TEST(tach_survives_micros_rollover) {
  TachEstimator t(2, 1500000);
  // Pulses at 50 Hz straddling the 32-bit microsecond wrap (~71.6 min).
  uint32_t us = 0xFFFFFFFFu - 300000u, count = 0;
  for (int i = 0; i < 25; i++) { us += 20000; count++; }
  t.update(count, us, us + 1000);
  for (int i = 0; i < 25; i++) { us += 20000; count++; }
  CHECK_NEAR(t.update(count, us, us + 1000), 1500, 1);
}

TEST(smoothing) {
  CHECK_EQ(smoothRpm(0, 1500, 4), 1500);   // start: snap
  CHECK_EQ(smoothRpm(1500, 0, 4), 0);      // stop: snap
  CHECK_EQ(smoothRpm(1000, 2000, 4), 1250);
  CHECK_EQ(smoothRpm(1000, 1002, 4), 1001);  // never stalls short of target
  CHECK_EQ(smoothRpm(1000, 998, 4), 999);
  CHECK_EQ(smoothRpm(1000, 2000, 1), 2000);
  uint16_t v = 1000;
  for (int i = 0; i < 60; i++) v = smoothRpm(v, 1800, 4);
  CHECK_EQ(v, 1800);
}

TEST(rpm_percent) {
  CHECK_EQ(rpmToPercent(0, 2000), 0);
  CHECK_EQ(rpmToPercent(1000, 2000), 50);
  CHECK_EQ(rpmToPercent(1990, 2000), 100);  // rounds
  CHECK_EQ(rpmToPercent(2500, 2000), 100);  // clamps
  CHECK_EQ(rpmToPercent(500, 0), 0);
  CHECK_EQ(rpmToPercent(65535, 100), 100);
}

TEST(decoder_half_step_detents) {
  // EC11 with 15 pulses / 30 detents: rest at 11 and at 00.
  QuadratureDecoder d(2);
  CHECK_EQ(d.update(0, 1), 0);
  CHECK_EQ(d.update(0, 0), 1);   // first click (rest at 00)
  CHECK_EQ(d.update(1, 0), 0);
  CHECK_EQ(d.update(1, 1), 1);   // second click (rest at 11)
  CHECK_EQ(d.update(1, 0), 0);
  CHECK_EQ(d.update(0, 0), -1);  // back one click
  // The default (full-cycle) decoder would ignore the 00 rest.
  QuadratureDecoder full;
  CHECK_EQ(full.update(0, 1), 0);
  CHECK_EQ(full.update(0, 0), 0);
}

TEST(decoder_rapid_direction_changes) {
  // Alternate full detents CW / CCW as fast as the edges arrive: every
  // detent must be reported with the correct sign, none lost or doubled.
  QuadratureDecoder d;
  const int cw[][2] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
  const int ccw[][2] = {{1, 0}, {0, 0}, {0, 1}, {1, 1}};
  int net = 0, count = 0;
  for (int i = 0; i < 50; i++) {
    const int a = feed(d, (i % 2) ? ccw : cw, 4);
    CHECK_EQ(a, (i % 2) ? -1 : 1);
    net += a;
    count += a != 0;
  }
  CHECK_EQ(net, 0);
  CHECK_EQ(count, 50);
  // Reversing in the middle of a detent produces nothing (not a false step).
  const int reverse[][2] = {{0, 1}, {0, 0}, {0, 1}, {1, 1}};
  CHECK_EQ(feed(d, reverse, 4), 0);
  // ... and the next full detent counts normally.
  CHECK_EQ(feed(d, ccw, 4), -1);
}

TEST(detents_at_limits) {
  // Encoder at min/max: further turns in the same direction stay clamped,
  // and the first turn back moves off the limit immediately.
  uint8_t s = 100;
  for (int i = 0; i < 10; i++) s = applyDetents(s, +1, 1000, 40, 1, 5, 0, 100);
  CHECK_EQ(s, 100);
  s = applyDetents(s, +100, 0, 40, 1, 5, 0, 100);  // huge fast burst
  CHECK_EQ(s, 100);
  s = applyDetents(s, -1, 1000, 40, 1, 5, 0, 100);
  CHECK_EQ(s, 99);
  s = 0;
  s = applyDetents(s, -100, 0, 40, 1, 5, 0, 100);
  CHECK_EQ(s, 0);
  s = applyDetents(s, +1, 1000, 40, 1, 5, 0, 100);
  CHECK_EQ(s, 1);
  // Custom range.
  CHECK_EQ(applyDetents(50, -100, 0, 40, 1, 5, 20, 80), 20);
  CHECK_EQ(applyDetents(50, +100, 0, 40, 1, 5, 20, 80), 80);
}

TEST(duty_mapping_guards) {
  // Misconfigured limits never produce more than 100 % or wrap around.
  CHECK_EQ(settingToDutyPermille(100, true, 0, 150), 1000);
  CHECK_EQ(settingToDutyPermille(50, true, 80, 20), 200);  // min > max -> min = max
  CHECK_EQ(settingToDutyPermille(255, true, 0, 100), 1000);
  // gate drive never exceeds its limits either
  GateDrive g = gateDriveFor(5000, 3999, true);
  CHECK(g.mode == GateDrive::ConstantLow);  // > 1000 treated as 1000 = full speed
  CHECK_EQ(timer1TopFor(16000000UL, 0, 4000), 3999);  // prescaler 0 guarded
  CHECK_EQ(timer1TopFor(16000000UL, 1, 0), 65535);    // 0 Hz guarded
}

TEST(polarity_inversion) {
  // Inverted (Cloudline): gate duty = 1000 - line duty.
  // Non-inverted: gate duty = line duty. Check the same line duties both ways.
  const uint16_t top = 3999;  // 4 kHz
  for (uint16_t d = 1; d < 1000; d += 37) {
    const GateDrive inv = gateDriveFor(d, top, true);
    const GateDrive non = gateDriveFor(d, top, false);
    CHECK(inv.mode == GateDrive::Pwm);
    CHECK(non.mode == GateDrive::Pwm);
    // high ticks of the two add up to one period (+/- rounding)
    const long sum = (long)inv.compare + 1 + (long)non.compare + 1;
    CHECK_NEAR(sum, top + 1, 2);
  }
  CHECK(gateDriveFor(0, top, true).mode == GateDrive::ConstantHigh);    // stopped
  CHECK(gateDriveFor(0, top, false).mode == GateDrive::ConstantLow);    // stopped
  CHECK(gateDriveFor(1000, top, true).mode == GateDrive::ConstantLow);  // full
  CHECK(gateDriveFor(1000, top, false).mode == GateDrive::ConstantHigh);
}

TEST(tach_fault_detection) {
  // Not running: never a fault.
  CHECK(!tachFault(0, 100000, 0, 0, 8000));
  // Just started, no pulses yet: no fault until faultMs has passed.
  CHECK(!tachFault(500, 7000, 0, 0, 8000));
  CHECK(tachFault(500, 9000, 0, 0, 8000));
  // Pulses recently: no fault.
  CHECK(!tachFault(500, 100000, 99000, 0, 8000));
  // Pulses stopped long ago, fan restarted recently: no fault yet.
  CHECK(!tachFault(500, 100000, 1000, 95000, 8000));
  // Across the millis() wrap.
  CHECK(!tachFault(500, 2000, 0xFFFFF000u, 0xFFFFF000u, 8000));
  CHECK(tachFault(500, 9000, 0xFFFFF000u, 0xFFFFF000u, 8000));
}

TEST(rpm_percent_max_not_calibrated) {
  // Divide-by-zero guard: an unknown maximum gives 0, never a crash.
  CHECK_EQ(rpmToPercent(1500, 0), 0);
  CHECK_EQ(rpmToPercent(0, 0), 0);
  CHECK_EQ(rpmToPercent(65535, 1), 100);
  CHECK_EQ(rpmToPercent(65535, 65535), 100);
}

static ControllerConfig testConfig() {
  ControllerConfig c;
  c.settingDefault = 50;
  c.settingMin = 0;
  c.settingMax = 100;
  c.stepSlow = 1;
  c.stepFast = 5;
  c.fastMs = 40;
  c.dutyMinPct = 0;
  c.dutyMaxPct = 100;
  c.maxRpm = 0;
  c.calSpinupMs = 12000;
  c.calSampleMs = 6000;
  c.calMinValidRpm = 200;
  c.calResultShowMs = 4000;
  return c;
}

TEST(controller_power_up_state) {
  Controller c(testConfig());
  c.begin(false);
  CHECK(!c.on());
  CHECK_EQ(c.setting(), 50);
  CHECK(c.mode() == Mode::Normal);
  CHECK_EQ(c.dutyPermille(), 0);  // OFF at power-up
  CHECK(!c.calibrated());
  // Nothing happens with time alone.
  for (uint32_t t = 0; t < 60000; t += 100) CHECK(!c.tick(t));
  CHECK_EQ(c.dutyPermille(), 0);
  // Knob held at boot: diagnostics, still OFF.
  Controller d(testConfig());
  d.begin(true);
  CHECK(d.mode() == Mode::Diagnostics);
  CHECK_EQ(d.dutyPermille(), 0);
}

TEST(controller_on_off_keeps_setting) {
  Controller c(testConfig());
  c.begin(false);
  CHECK(c.onDetents(+10, 1000));
  CHECK_EQ(c.setting(), 60);
  CHECK_EQ(c.dutyPermille(), 0);  // turning while OFF does not start the fan
  CHECK(c.onKnob(ButtonEvent::Short, 2000));
  CHECK(c.on());
  CHECK_EQ(c.dutyPermille(), 600);
  c.onKnob(ButtonEvent::Short, 3000);
  CHECK(!c.on());
  CHECK_EQ(c.dutyPermille(), 0);
  CHECK_EQ(c.setting(), 60);       // kept while powered
  c.onKnob(ButtonEvent::Short, 4000);
  CHECK_EQ(c.dutyPermille(), 600);
  // ON at 0 % is still stopped (duty 0).
  c.onDetents(-100, 5000);
  CHECK_EQ(c.setting(), 0);
  CHECK(c.on());
  CHECK_EQ(c.dutyPermille(), 0);
  CHECK(!c.onKnob(ButtonEvent::None, 6000));
  CHECK(!c.onDetents(0, 6000));
}

TEST(controller_min_duty_mapping) {
  ControllerConfig cfg = testConfig();
  cfg.dutyMinPct = 30;
  Controller c(cfg);
  c.setOn(true);
  c.setSetting(1);
  CHECK_EQ(c.dutyPermille(), 307);  // 300 + 700 * 1 / 100
  c.setSetting(100);
  CHECK_EQ(c.dutyPermille(), 1000);
  c.setSetting(0);
  CHECK_EQ(c.dutyPermille(), 0);
  c.setSetting(200);
  CHECK_EQ(c.setting(), 100);  // clamped
}

TEST(controller_calibration_success) {
  Controller c(testConfig());
  c.setOn(true);
  c.setSetting(40);
  CHECK(c.onKnob(ButtonEvent::Long, 1000));
  CHECK(c.mode() == Mode::Calibrating);
  CHECK_EQ(c.dutyPermille(), 1000);  // forced to 100 %
  CHECK(!c.onDetents(+5, 1100) || c.setting() == 40);  // knob ignored
  CHECK_EQ(c.setting(), 40);
  // Samples during spin-up are ignored; afterwards they are averaged.
  for (uint32_t t = 1000; t <= 1000 + 18000; t += 500) {
    c.onTachSample(t < 13000 ? 500 : 1800 + (t / 500 % 3), t);  // 1800..1802
    c.tick(t);
  }
  CHECK(c.mode() == Mode::CalResult);
  CHECK(c.lastCalOk());
  CHECK_NEAR(c.maxRpm(), 1801, 1);
  CHECK(c.calibrated());
  CHECK_EQ(c.dutyPermille(), 400);  // back to the setting
  // Result screen times out.
  CHECK(c.tick(19000 + 4000));
  CHECK(c.mode() == Mode::Normal);
}

TEST(controller_calibration_rejects_unsteady_or_missing) {
  Controller c(testConfig());
  c.setOn(true);
  CHECK(c.startCalibration(0));
  for (uint32_t t = 0; t <= 18000; t += 500) {
    c.onTachSample((t / 500) % 2 ? 1500 : 2000, t);  // swings +/-14 %
    c.tick(t);
  }
  CHECK(c.mode() == Mode::CalResult);
  CHECK(!c.lastCalOk());
  CHECK(!c.calibrated());
  CHECK_EQ(c.maxRpm(), 0);
  c.onKnob(ButtonEvent::Short, 18100);  // any press dismisses
  CHECK(c.mode() == Mode::Normal);
  // No tach at all.
  CHECK(c.startCalibration(20000));
  for (uint32_t t = 20000; t <= 38000; t += 500) { c.onTachSample(0, t); c.tick(t); }
  CHECK(!c.lastCalOk());
  // Below the minimum valid RPM.
  c.onBakKey(ButtonEvent::Short);
  CHECK(c.startCalibration(40000));
  for (uint32_t t = 40000; t <= 58000; t += 500) { c.onTachSample(150, t); c.tick(t); }
  CHECK(!c.lastCalOk());
}

TEST(controller_calibration_rule_is_plus_minus_10_percent) {
  // One sample 17 % below the average must be rejected (each sample within
  // +/-10 % of the average), even though the total spread is under 20 %.
  Controller c(testConfig());
  c.setOn(true);
  CHECK(c.startCalibration(0));
  int n = 0;
  for (uint32_t t = 12000; t <= 18000; t += 500, n++) c.onTachSample(n == 0 ? 1640 : 2000, t);
  c.tick(18000);
  CHECK(c.mode() == Mode::CalResult);
  CHECK(!c.lastCalOk());
  // Samples within +/-5 %: accepted.
  c.onKnob(ButtonEvent::Short, 18100);
  CHECK(c.startCalibration(20000));
  n = 0;
  for (uint32_t t = 32000; t <= 38000; t += 500, n++) c.onTachSample(n % 2 ? 1950 : 2050, t);
  c.tick(38000);
  CHECK(c.lastCalOk());
  CHECK_NEAR(c.maxRpm(), 2000, 5);
}

TEST(controller_calibration_guards_and_cancel) {
  Controller c(testConfig());
  // OFF: neither a long press nor a command starts it (never starts the fan).
  CHECK(!c.onKnob(ButtonEvent::Long, 0));
  CHECK(!c.startCalibration(0));
  CHECK_EQ(c.dutyPermille(), 0);
  c.setOn(true);
  // Not from the diagnostics page.
  c.onConKey(ButtonEvent::Short);
  CHECK(c.mode() == Mode::Diagnostics);
  CHECK(!c.onKnob(ButtonEvent::Long, 0));
  c.onBakKey(ButtonEvent::Short);
  CHECK(c.mode() == Mode::Normal);
  // Cancel by knob press, BAK, command, or switching OFF.
  CHECK(c.startCalibration(0));
  CHECK(c.onKnob(ButtonEvent::Short, 100));
  CHECK(c.mode() == Mode::Normal);
  CHECK(c.on());  // cancelling does not toggle the fan
  CHECK(c.startCalibration(0));
  CHECK(c.onBakKey(ButtonEvent::Short));
  CHECK(c.mode() == Mode::Normal);
  CHECK(c.startCalibration(0));
  CHECK(c.cancelCalibration());
  CHECK(!c.cancelCalibration());
  CHECK(c.startCalibration(0));
  c.setOn(false);
  CHECK(c.mode() == Mode::Normal);
  CHECK_EQ(c.dutyPermille(), 0);
}

TEST(controller_screens) {
  Controller c(testConfig());
  CHECK(c.onConKey(ButtonEvent::Short));
  CHECK(c.mode() == Mode::Diagnostics);
  CHECK(c.onKnob(ButtonEvent::Short, 0));  // toggles on this page too
  CHECK(c.on());
  CHECK(c.onDetents(-5, 1000));            // and adjusts the setting (slow: 1 %/click)
  CHECK_EQ(c.setting(), 45);
  CHECK(c.onConKey(ButtonEvent::Short));
  CHECK(c.mode() == Mode::Normal);
  CHECK(!c.onBakKey(ButtonEvent::Short));  // BAK on main does nothing
  CHECK(!c.onConKey(ButtonEvent::Long));
  c.setMaxRpm(1800);
  CHECK(c.calibrated());
  c.setMaxRpm(0);
  CHECK(!c.calibrated());
}

TEST(command_parser) {
  CHECK(parseCommand("").cmd == Cmd::None);
  CHECK(parseCommand("   ").cmd == Cmd::None);
  CHECK(parseCommand("?").cmd == Cmd::Help);
  CHECK(parseCommand("HELP").cmd == Cmd::Help);
  CHECK(parseCommand("s").cmd == Cmd::Status);
  CHECK(parseCommand(" on ").cmd == Cmd::On);
  CHECK(parseCommand("Off").cmd == Cmd::Off);
  Command c = parseCommand("set 42");
  CHECK(c.cmd == Cmd::Set);
  CHECK_EQ(c.value, 42);
  c = parseCommand("m 65535");
  CHECK(c.cmd == Cmd::MaxRpm);
  CHECK_EQ(c.value, 65535);
  CHECK(parseCommand("m 65536").cmd == Cmd::Invalid);  // overflow
  CHECK(parseCommand("set").cmd == Cmd::Invalid);       // missing number
  CHECK(parseCommand("on 5").cmd == Cmd::Invalid);      // unexpected number
  CHECK(parseCommand("set 4x").cmd == Cmd::Invalid);    // trailing garbage
  CHECK(parseCommand("set -1").cmd == Cmd::Invalid);
  CHECK(parseCommand("calibrate").cmd == Cmd::Invalid); // word too long
  CHECK(parseCommand("c").cmd == Cmd::Cal);
  CHECK(parseCommand("x").cmd == Cmd::Cancel);
  CHECK(parseCommand("zz").cmd == Cmd::Invalid);
}

int main() {
  RUN(decoder_counts_one_detent_per_full_cycle);
  RUN(decoder_emits_only_at_rest_position);
  RUN(decoder_ignores_bounce_and_aborted_turns);
  RUN(decoder_discards_invalid_double_jump);
  RUN(decoder_reset_resynchronises);
  RUN(button_short_press_after_debounce);
  RUN(button_ignores_glitches);
  RUN(button_long_press_fires_once_and_suppresses_short);
  RUN(button_suppressed_press_produces_nothing);
  RUN(button_bounce_gives_one_press);
  RUN(detents_step_accelerate_and_clamp);
  RUN(setting_to_duty_mapping);
  RUN(gate_drive_inverted);
  RUN(gate_drive_non_inverted);
  RUN(timer1_top);
  RUN(tach_estimates_rpm_from_periods);
  RUN(tach_reports_zero_after_timeout);
  RUN(tach_pulses_per_rev);
  RUN(tach_survives_micros_rollover);
  RUN(smoothing);
  RUN(rpm_percent);
  RUN(decoder_half_step_detents);
  RUN(decoder_rapid_direction_changes);
  RUN(detents_at_limits);
  RUN(duty_mapping_guards);
  RUN(polarity_inversion);
  RUN(tach_fault_detection);
  RUN(rpm_percent_max_not_calibrated);
  RUN(controller_power_up_state);
  RUN(controller_on_off_keeps_setting);
  RUN(controller_min_duty_mapping);
  RUN(controller_calibration_success);
  RUN(controller_calibration_rejects_unsteady_or_missing);
  RUN(controller_calibration_rule_is_plus_minus_10_percent);
  RUN(controller_calibration_guards_and_cancel);
  RUN(controller_screens);
  RUN(command_parser);
  return summary("fan_logic");
}
