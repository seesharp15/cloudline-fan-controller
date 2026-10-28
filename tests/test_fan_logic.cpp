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

TEST(cal_record) {
  CalRecord r = makeCalRecord(1834);
  CHECK(calRecordValid(r));
  CalRecord erased = {0xFFFF, 0xFFFF, 0xFFFF};
  CHECK(!calRecordValid(erased));
  CalRecord zero = {0, 0, 0};
  CHECK(!calRecordValid(zero));
  r.maxRpm ^= 1;  // corrupted
  CHECK(!calRecordValid(r));
  CHECK(!calRecordValid(makeCalRecord(50)));  // implausible
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
  RUN(cal_record);
  return summary("fan_logic");
}
