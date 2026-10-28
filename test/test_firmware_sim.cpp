// End-to-end simulation of the real firmware on the host.
//
// app.cpp is compiled unchanged against the stubs in test/stubs/. Each run
// of this program is one "power-up" (global constructors run once per
// process), so the Makefile runs one process per scenario:
//
//   test_firmware_sim boot1 [screenshot-dir]  main scenario: knob, buttons,
//                                             tach, calibration, diagnostics
//   test_firmware_sim powercycle              what a fresh power-up keeps
//   test_firmware_sim serial                  serial console commands
//   test_firmware_sim oledaddr                display at 0x3D: console report
//   test_firmware_sim diagboot                knob held at power-up
//   test_firmware_sim wrap                    across the millis() rollover
//   test_firmware_sim calfail [screenshot-dir] calibration with no tach

#include "stubs/Arduino.h"
#include "stubs/U8g2lib.h"

#include "../firmware/cloudline_controller/app.cpp"

#include <stdlib.h>
#include <string.h>

#include <string>

#include "check.h"
#include "sim.h"

using fan::Mode;

static const char *g_shotDir = nullptr;
static unsigned g_framesChecked = 0;

// Check the last frame sent for layout problems, then optionally save it.
static void checkFrame(const char *shotName = nullptr) {
  for (const std::string &p : oled.last.check()) {
    g_failures++;
    fprintf(stderr, "  FAIL layout: %s\n     frame: %s\n", p.c_str(), oled.last.allText().c_str());
  }
  g_checks++;
  g_framesChecked++;
  if (shotName && g_shotDir) {
    const std::string path = std::string(g_shotDir) + "/" + shotName + ".svg";
    CHECK(oled.last.writeSvg(path.c_str(), 4));
  }
}

// Advance in display-refresh steps, checking each frame's layout.
static void run(uint32_t ms) {
  for (uint32_t t = 0; t < ms; t += DISPLAY_REFRESH_MS) {
    sim::advance(DISPLAY_REFRESH_MS);
    checkFrame();
  }
}

// D9 constant HIGH = Q1 on = fan line held low = fan stopped.
static bool fanStopped() { return !sim::pwmConnected() && sim::pins[PIN_FAN_PWM].out == HIGH; }
static bool fanFull() { return !sim::pwmConnected() && sim::pins[PIN_FAN_PWM].out == LOW; }
// Duty on the fan's speed line = time Q1 is OFF = time the gate is LOW.
static int fanLineDuty() { return 1000 - sim::gateDutyPermille(); }
static bool serialHas(const std::string &out, const char *needle) {
  return out.find(needle) != std::string::npos;
}

static void powerUp() {
  sim::reset();
  // Before setup() runs, D9 is a floating input: only R4 keeps the fan off.
  CHECK_EQ(sim::pins[PIN_FAN_PWM].mode, INPUT);
  setup();
}

// Checks shared by every power-up.
static void checkPowerUpState() {
  CHECK(fanStopped());
  CHECK_EQ(sim::pins[PIN_FAN_PWM].mode, OUTPUT);
  // The gate was already HIGH (fan stopped) when D9 became an output: no glitch.
  CHECK_EQ(sim::pins[PIN_FAN_PWM].outWhenMadeOutput, HIGH);
  CHECK(!gCtl.on());
  CHECK_EQ(gCtl.setting(), SETTING_DEFAULT_PCT);
  CHECK_EQ(gCtl.maxRpm(), TACH_MAX_RPM);
  CHECK(gWatchdogStarted == (WATCHDOG_ENABLE != 0));
}

static void boot1() {
  // --- Power-up: fan OFF, default setting, display initialised ----------
  checkPowerUpState();
  CHECK(oled.begun);
  CHECK_EQ(oled.i2cAddress, 0x78);
  CHECK_EQ(oled.busClock, 400000);
  CHECK(Wire.begun);
  CHECK_EQ(Wire.timeoutUs, 25000);   // I2C can't block forever
  CHECK(Wire.resetWithTimeout);
  CHECK_EQ(sim::pins[PIN_TACH].mode, INPUT_PULLUP);  // D2 defined even without R6
  CHECK_EQ(sim::pwmTop(), 3999);      // 4 kHz
  CHECK(PCICR & _BV(2));
  CHECK_EQ(PCMSK2, _BV(3) | _BV(4));
  CHECK(sim::extIsr[0] != nullptr);   // tach on INT0
  CHECK(sim::extIsr[1] == nullptr);   // INT1 unused (D3 is on PCINT2)
  CHECK_EQ(Serial.baud, 115200);
  {
    const std::string out = sim::takeSerial();
    CHECK(serialHas(out, "Fan OFF"));
    CHECK(serialHas(out, "OLED answers at 0x3C"));
  }
  run(500);
  CHECK(fanStopped());
  CHECK_EQ(sim::pins[PIN_STATUS_LED].out, LOW);
  CHECK(oled.last.hasText("FAN OFF"));
  CHECK(oled.last.hasText("50%"));
  CHECK(oled.last.hasText("0 RPM"));
  CHECK(oled.last.hasText("Press knob to start"));
  checkFrame("screen_off");

  // --- Turning while OFF changes the setting but not the fan -------------
  sim::turn(5, 100);
  run(200);
  CHECK_EQ(gCtl.setting(), 55);
  CHECK(fanStopped());
  // Fast spin: 5 % per detent. The first detent of a spin follows a pause,
  // so it is still a 1 % step: 55 + 1 + 5 + 5.
  sim::turn(3, 10);
  CHECK_EQ(gCtl.setting(), 66);
  sim::turn(-80, 60);
  CHECK_EQ(gCtl.setting(), 0);  // clamps at the bottom
  sim::turn(-3, 60);
  CHECK_EQ(gCtl.setting(), 0);
  sim::turn(30, 60);
  CHECK_EQ(gCtl.setting(), 30);
  sim::bounceDetent(+1);  // chattering contacts still count once
  CHECK_EQ(gCtl.setting(), 31);
  sim::bounceDetent(-1);
  CHECK_EQ(gCtl.setting(), 30);
  sim::turn(20, 60);
  CHECK_EQ(gCtl.setting(), 50);
  CHECK(fanStopped());

  // --- Short press: fan ON at the setting ---------------------------------
  sim::press(PIN_ENC_SW, 120);
  CHECK(gCtl.on());
  CHECK(sim::pwmConnected());
  CHECK_NEAR(fanLineDuty(), 500, 1);
  CHECK_EQ(sim::pins[PIN_STATUS_LED].out, HIGH);
  // The fan spins up: 50 Hz at 2 pulses/rev = 1500 RPM. Max RPM is not
  // known yet, so ACT shows "--%" and the footer asks for a calibration.
  sim::tachHz = 50;
  run(3000);
  CHECK_NEAR(gRpm, 1500, 3);
  CHECK(oled.last.hasText("FAN ON"));
  CHECK(oled.last.hasText("1500 RPM") || oled.last.hasText("1499 RPM") || oled.last.hasText("1501 RPM"));
  CHECK(oled.last.hasText("--%"));
  CHECK(oled.last.hasText("Hold knob 3s: CAL"));
  checkFrame("screen_on_uncalibrated");

  // --- Setting changes apply immediately while ON -------------------------
  sim::turn(-25, 60);
  CHECK_EQ(gCtl.setting(), 25);
  CHECK_NEAR(fanLineDuty(), 250, 1);
  sim::turn(75, 60);
  CHECK_EQ(gCtl.setting(), 100);
  CHECK(fanFull());
  sim::turn(-30, 60);
  CHECK_NEAR(fanLineDuty(), 700, 1);

  // --- OFF keeps the setting; ON restores it --------------------------------
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gCtl.on());
  CHECK(fanStopped());
  CHECK_EQ(gCtl.setting(), 70);
  sim::tachHz = 0;
  run(2500);
  CHECK_EQ(gRpm, 0);  // tach timeout -> 0 RPM
  sim::press(PIN_ENC_SW, 100);
  CHECK(gCtl.on());
  CHECK_NEAR(fanLineDuty(), 700, 1);

  // --- Tach missing while ON -> warning, fan keeps running ---------------
  run(TACH_FAULT_MS + 1000);
  CHECK(oled.last.hasText("NO TACH"));
  CHECK(oled.last.hasText("--%"));
  CHECK(sim::pwmConnected());
  checkFrame("screen_no_tach");
  sim::tachHz = 46.667;  // 1400 RPM
  run(2000);
  CHECK(!oled.last.hasText("NO TACH"));
  CHECK_NEAR(gRpm, 1400, 3);

  // --- Long press while ON: calibration ------------------------------------
  sim::takeSerial();
  sim::hold(PIN_ENC_SW);
  run(1500);
  CHECK(oled.last.hasText("Hold for CAL: 2s"));
  checkFrame("screen_hold_cal");
  run(1600);
  CHECK(gCtl.mode() == Mode::Calibrating);
  CHECK(fanFull());  // calibration runs at 100 %
  sim::release(PIN_ENC_SW);
  run(200);
  CHECK(gCtl.mode() == Mode::Calibrating);  // releasing the long press does not cancel
  sim::tachHz = 61.2;  // 1836 RPM at full speed
  run(6000);
  CHECK(oled.last.hasText("CALIBRATE"));
  CHECK(oled.last.hasText("Spin-up"));
  checkFrame("screen_calibrating");
  sim::turn(5, 60);  // knob is ignored while calibrating
  CHECK_EQ(gCtl.setting(), 70);
  // ~6.6 s of the 18 s run have elapsed; finish it but stay within the
  // time the result is shown for.
  run(CAL_SPINUP_MS + CAL_SAMPLE_MS - 6000);
  CHECK(gCtl.mode() == Mode::CalResult);
  CHECK(gCtl.lastCalOk());
  CHECK_NEAR(gCtl.lastCalRpm(), 1836, 3);
  CHECK(gCtl.calibrated());
  CHECK_NEAR(gCtl.maxRpm(), 1836, 3);
  CHECK(oled.last.hasText("CAL DONE"));
  CHECK(oled.last.hasText("Kept until power-off."));
  checkFrame("screen_cal_done");
  {
    const std::string out = sim::takeSerial();
    CHECK(serialHas(out, "CAL OK"));
    CHECK(serialHas(out, "#define TACH_MAX_RPM 18"));
  }
  CHECK_NEAR(fanLineDuty(), 700, 1);  // result screen: back to the setting
  run(CAL_RESULT_SHOW_MS + 200);
  CHECK(gCtl.mode() == Mode::Normal);
  CHECK_NEAR(fanLineDuty(), 700, 1);
  sim::tachHz = 61.2 * 0.7;  // 70 % of the calibrated maximum
  run(3000);
  CHECK(oled.last.hasText("70%"));
  CHECK(!oled.last.hasText("Hold knob 3s"));  // calibrated now
  CHECK(oled.last.hasText("Turn knob: speed"));
  sim::turn(-20, 60);
  sim::tachHz = 61.2 * 0.5 * 0.97;  // fan a little slower than commanded
  run(8000);  // let the display smoothing settle
  CHECK(oled.last.hasText("SET"));
  CHECK(oled.last.hasText("49%"));
  checkFrame("screen_on");
  sim::turn(20, 60);

  // --- Calibration can be cancelled with a fresh press ---------------------
  sim::hold(PIN_ENC_SW);
  run(CAL_HOLD_MS + 200);
  sim::release(PIN_ENC_SW);
  run(500);
  CHECK(gCtl.mode() == Mode::Calibrating);
  sim::press(PIN_ENC_SW, 100);
  CHECK(gCtl.mode() == Mode::Normal);
  CHECK(gCtl.on());
  CHECK_NEAR(fanLineDuty(), 700, 1);

  // --- Long press while OFF does nothing -----------------------------------
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gCtl.on());
  sim::hold(PIN_ENC_SW);
  run(CAL_HOLD_MS + 500);
  sim::release(PIN_ENC_SW);
  run(200);
  CHECK(gCtl.mode() == Mode::Normal);
  CHECK(!gCtl.on());
  CHECK(fanStopped());

  // --- Button bounce: one toggle per physical press -------------------------
  for (int i = 0; i < 6; i++) {  // 6 contact bounces 2 ms apart, then held
    sim::hold(PIN_ENC_SW);
    sim::advance(2);
    sim::release(PIN_ENC_SW);
    sim::advance(2);
  }
  sim::hold(PIN_ENC_SW);
  sim::advance(100);
  for (int i = 0; i < 4; i++) {  // release bounce
    sim::release(PIN_ENC_SW);
    sim::advance(2);
    sim::hold(PIN_ENC_SW);
    sim::advance(2);
  }
  sim::release(PIN_ENC_SW);
  sim::advance(100);
  CHECK(gCtl.on());  // exactly one toggle: OFF -> ON
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gCtl.on());

  // --- Diagnostics page via CON, back via BAK ------------------------------
  sim::press(PIN_ENC_SW, 100);  // ON again
  sim::tachHz = 42.8;
  run(2000);
  sim::press(PIN_KEY_CON, 100);
  run(300);
  CHECK(gCtl.mode() == Mode::Diagnostics);
  CHECK(oled.last.hasText("DIAG"));
  CHECK(oled.last.hasText("Hz"));
  CHECK(oled.last.hasText("Gate PWM"));
  checkFrame("screen_diagnostics");
  sim::press(PIN_ENC_SW, 100);  // knob still toggles the fan on this page
  CHECK(!gCtl.on());
  sim::press(PIN_KEY_BAK, 100);
  CHECK(gCtl.mode() == Mode::Normal);

  // --- Display cadence: 10 Hz, each frame drawn in 4 page passes -----------
  const unsigned before = oled.frames;
  run(10000);
  CHECK(oled.frames - before >= 95);
  CHECK(oled.frames - before <= 105);
  CHECK_EQ(oled.outOfBounds, 0);
  // The watchdog is fed on every pass of loop() (once per simulated ms).
  const unsigned long feeds = g_wdtFeeds;
  sim::advance(1000);
  CHECK(g_wdtFeeds - feeds >= 1000);
}

static void powercycle() {
  // A fresh power-up keeps nothing: fan OFF, default setting, max RPM back
  // to the config value (calibration was RAM only). There is no EEPROM code
  // in the firmware at all (the Makefile also greps for it).
  checkPowerUpState();
  CHECK(!gCtl.calibrated());
  run(1000);
  CHECK(fanStopped());
  CHECK(oled.last.hasText("FAN OFF"));
  CHECK(oled.last.hasText("50%"));
  CHECK_EQ(oled.outOfBounds, 0);
}

static void serialScenario() {
  checkPowerUpState();
  sim::takeSerial();
  sim::serial("?");
  CHECK(serialHas(sim::takeSerial(), "Commands"));
  sim::serial("set 30");
  CHECK_EQ(gCtl.setting(), 30);
  CHECK(fanStopped());  // setting alone does not start the fan
  CHECK(serialHas(sim::takeSerial(), "on=0 set=30%"));
  sim::serial("ON");
  CHECK(gCtl.on());
  CHECK_NEAR(fanLineDuty(), 300, 1);
  sim::serial("set 250");
  CHECK_EQ(gCtl.setting(), 100);  // clamped
  sim::serial("c");               // calibration needs no max RPM in advance
  CHECK(gCtl.mode() == Mode::Calibrating);
  sim::serial("x");
  CHECK(gCtl.mode() == Mode::Normal);
  sim::serial("m 1800");
  CHECK_EQ(gCtl.maxRpm(), 1800);
  sim::tachHz = 30;  // 900 RPM = 50 %
  run(3000);
  sim::takeSerial();
  sim::serial("s");
  {
    const std::string out = sim::takeSerial();
    CHECK(serialHas(out, "on=1"));
    CHECK(serialHas(out, "act=50%"));
    CHECK(serialHas(out, "max=1800"));
  }
  sim::serial("bogus");
  CHECK(serialHas(sim::takeSerial(), "unknown command"));
  sim::serial("set");  // missing number
  CHECK(serialHas(sim::takeSerial(), "unknown command"));
  // A line longer than the buffer is truncated, not overflowed.
  sim::serial("set 1111111111111111111111111111111111111111");
  CHECK(serialHas(sim::takeSerial(), "unknown command"));
  sim::serial("off");
  CHECK(!gCtl.on());
  CHECK(fanStopped());
  sim::serial("c");  // refused while OFF: calibration never starts the fan
  CHECK(gCtl.mode() == Mode::Normal);
  CHECK(fanStopped());
  CHECK(serialHas(sim::takeSerial(), "needs the fan ON"));
}

static void oledaddr() {
  // Display at the other address: the console says so, nothing hangs.
  checkPowerUpState();
  const std::string out = sim::takeSerial();
  CHECK(serialHas(out, "! no I2C device at 0x3C"));
  CHECK(serialHas(out, "answers at 0x3D"));
  run(500);
  CHECK(fanStopped());
}

static void diagboot() {
  // Knob held while powering up: diagnostics page, and that press neither
  // toggles the fan nor starts a calibration.
  checkPowerUpState();
  CHECK(gCtl.mode() == Mode::Diagnostics);
  run(CAL_HOLD_MS + 1000);
  sim::release(PIN_ENC_SW);
  run(300);
  CHECK(!gCtl.on());
  CHECK(fanStopped());
  CHECK(gCtl.mode() == Mode::Diagnostics);
  sim::press(PIN_ENC_SW, 100);  // the next press works
  CHECK(gCtl.on());
}

static void wrap() {
  // Powered up 10 s before millis() wraps (49.7 days of uptime in practice;
  // micros() wraps 1000x as often). Everything must keep working across it.
  checkPowerUpState();
  sim::press(PIN_ENC_SW, 100);
  CHECK(gCtl.on());
  sim::tachHz = 50;
  run(4000);
  CHECK_NEAR(gRpm, 1500, 3);
  run(12000);  // crosses the wrap
  CHECK(millis() < 10000u);
  CHECK_NEAR(gRpm, 1500, 3);
  CHECK(!oled.last.hasText("NO TACH"));
  sim::turn(-10, 60);
  CHECK_EQ(gCtl.setting(), 40);
  CHECK_NEAR(fanLineDuty(), 400, 1);
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gCtl.on());
  CHECK(fanStopped());
  const unsigned before = oled.frames;
  run(2000);
  CHECK(oled.frames - before >= 19);  // display still refreshing
}

static void calfail() {
  checkPowerUpState();
  sim::press(PIN_ENC_SW, 100);
  CHECK(gCtl.on());
  sim::hold(PIN_ENC_SW);
  run(CAL_HOLD_MS + 200);
  sim::release(PIN_ENC_SW);
  CHECK(gCtl.mode() == Mode::Calibrating);
  run(CAL_SPINUP_MS + CAL_SAMPLE_MS + 200);  // no tach at all
  CHECK(gCtl.mode() == Mode::CalResult);
  CHECK(!gCtl.lastCalOk());
  CHECK(!gCtl.calibrated());
  CHECK_EQ(gCtl.maxRpm(), TACH_MAX_RPM);  // unchanged
  CHECK(oled.last.hasText("CAL FAILED"));
  CHECK(serialHas(sim::takeSerial(), "CAL FAILED"));
  checkFrame("screen_cal_failed");
  sim::press(PIN_KEY_BAK, 100);
  CHECK(gCtl.mode() == Mode::Normal);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s boot1|powercycle|serial|oledaddr|diagboot|wrap|calfail [shot-dir]\n", argv[0]);
    return 2;
  }
  const char *scenario = argv[1];
  g_shotDir = argc > 2 ? argv[2] : nullptr;

  if (!strcmp(scenario, "boot1")) {
    powerUp();
    boot1();
  } else if (!strcmp(scenario, "powercycle")) {
    powerUp();
    powercycle();
  } else if (!strcmp(scenario, "serial")) {
    powerUp();
    serialScenario();
  } else if (!strcmp(scenario, "oledaddr")) {
    sim::reset();
    Wire.present[0x3C] = false;
    Wire.present[0x3D] = true;
    setup();
    oledaddr();
  } else if (!strcmp(scenario, "diagboot")) {
    sim::reset();
    sim::hold(PIN_ENC_SW);
    setup();
    diagboot();
  } else if (!strcmp(scenario, "wrap")) {
    sim::reset();
    sim::nowUs = (uint64_t)0xFFFFFFFFu * 1000u - 10000000u;  // 10 s before millis() wraps
    setup();
    wrap();
  } else if (!strcmp(scenario, "calfail")) {
    powerUp();
    calfail();
  } else {
    fprintf(stderr, "unknown scenario %s\n", scenario);
    return 2;
  }
  printf("%s: %u frames layout-checked\n", scenario, g_framesChecked);
  return summary(scenario);
}
