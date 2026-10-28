// End-to-end simulation of the real sketch on the host.
//
// The .ino is compiled unchanged against the stubs in tests/stubs/. Each run
// of this program is one "power-up"; the Makefile runs it several times with
// a shared EEPROM file to check what survives a power cycle.
//
//   test_firmware_sim boot1 <eeprom> [screenshot-dir]   main scenario
//   test_firmware_sim boot2 <eeprom>                    after a power cycle
//   test_firmware_sim diagboot <eeprom>                 knob held at power-up
//   test_firmware_sim wrap <eeprom>                     across millis() rollover
//   test_firmware_sim calfail <eeprom>                  calibration with no tach

#include "stubs/Arduino.h"
#include "stubs/EEPROM.h"
#include "stubs/U8g2lib.h"

#include "../firmware/cloudline_controller/cloudline_controller.ino"

#include <stdlib.h>
#include <string.h>

#include <string>

#include "check.h"
#include "sim.h"

static const char *g_shotDir = nullptr;
static unsigned g_framesChecked = 0;

// Check every frame sent so far for layout problems, then optionally save it.
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

static bool fanStopped() { return !sim::pwmConnected() && sim::pins[PIN_FAN_PWM].out == HIGH; }
static bool fanFull() { return !sim::pwmConnected() && sim::pins[PIN_FAN_PWM].out == LOW; }
// Duty on the fan's speed line = time Q1 is OFF = time the gate is LOW.
static int fanLineDuty() { return 1000 - sim::gateDutyPermille(); }

static void powerUp(const char *eeprom) {
  sim::reset();
  if (eeprom) sim::eepromLoad(eeprom);  // missing file = blank EEPROM
  setup();
}

static void boot1() {
  // --- Power-up: fan OFF, default setting, display initialised ----------
  CHECK(fanStopped());
  CHECK_EQ(sim::pins[PIN_FAN_PWM].mode, OUTPUT);
  CHECK(oled.begun);
  CHECK_EQ(oled.i2cAddress, 0x78);
  CHECK_EQ(oled.busClock, 400000);
  CHECK_EQ(sim::pins[SDA].out, LOW);  // internal 5 V pull-ups disabled
  CHECK_EQ(sim::pins[SCL].out, LOW);
  CHECK_EQ(sim::pwmTop(), 15999);     // 1 kHz
  CHECK(PCICR & _BV(2));
  CHECK_EQ(PCMSK2, _BV(3) | _BV(4));
  CHECK(sim::extIsr[0] != nullptr);
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
  CHECK_EQ(gSetting, 55);
  CHECK(fanStopped());
  // Fast spin: 5 % per detent. The first detent of a spin follows a pause,
  // so it is still a 1 % step: 55 + 1 + 5 + 5.
  sim::turn(3, 10);
  CHECK_EQ(gSetting, 66);
  sim::turn(-80, 60);
  CHECK_EQ(gSetting, 0);  // clamps
  sim::turn(-3, 60);
  CHECK_EQ(gSetting, 0);
  sim::turn(30, 60);
  CHECK_EQ(gSetting, 30);
  sim::bounceDetent(+1);  // chattering contacts still count once
  CHECK_EQ(gSetting, 31);
  sim::bounceDetent(-1);
  CHECK_EQ(gSetting, 30);
  sim::turn(20, 60);
  CHECK_EQ(gSetting, 50);
  CHECK(fanStopped());

  // --- Short press: fan ON at the setting ---------------------------------
  sim::press(PIN_ENC_SW, 120);
  CHECK(gFanOn);
  CHECK(sim::pwmConnected());
  CHECK_NEAR(fanLineDuty(), 500, 1);
  CHECK_EQ(sim::pins[PIN_STATUS_LED].out, HIGH);
  // The fan spins up: 50 Hz at 2 pulses/rev = 1500 RPM = 75 % of 2000.
  sim::tachHz = 50;
  run(3000);
  CHECK_NEAR(gRpm, 1500, 3);
  CHECK(oled.last.hasText("FAN ON"));
  CHECK(oled.last.hasText("1500 RPM") || oled.last.hasText("1499 RPM") || oled.last.hasText("1501 RPM"));
  CHECK(oled.last.hasText("75%"));
  CHECK(oled.last.hasText("Hold knob 3s: CAL"));
  checkFrame("screen_on");

  // --- Setting changes apply immediately while ON -------------------------
  sim::turn(-25, 60);
  CHECK_EQ(gSetting, 25);
  CHECK_NEAR(fanLineDuty(), 250, 1);
  sim::turn(75, 60);
  CHECK_EQ(gSetting, 100);
  CHECK(fanFull());
  sim::turn(-30, 60);
  CHECK_NEAR(fanLineDuty(), 700, 1);

  // --- OFF keeps the setting; ON restores it --------------------------------
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gFanOn);
  CHECK(fanStopped());
  CHECK_EQ(gSetting, 70);
  sim::tachHz = 0;
  run(2500);
  CHECK_EQ(gRpm, 0);
  sim::press(PIN_ENC_SW, 100);
  CHECK(gFanOn);
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
  CHECK(oled.last.hasText("70%"));

  // --- Long press while ON: calibration ------------------------------------
  sim::hold(PIN_ENC_SW);
  run(1500);
  CHECK(oled.last.hasText("Hold for CAL: 2s"));
  checkFrame("screen_hold_cal");
  run(1600);
  CHECK(gMode == Mode::Calibrating);
  CHECK(fanFull());  // calibration runs at 100 %
  sim::release(PIN_ENC_SW);
  run(200);
  CHECK(gMode == Mode::Calibrating);  // releasing the long press does not cancel
  sim::tachHz = 61.2;  // ~1836 RPM at full speed
  run(6000);
  CHECK(oled.last.hasText("CALIBRATE"));
  CHECK(oled.last.hasText("Spin-up"));
  checkFrame("screen_calibrating");
  sim::turn(5, 60);  // knob is ignored while calibrating
  CHECK_EQ(gSetting, 70);
  // ~6.6 s of the 18 s run have elapsed; finish it but stay within the 3 s
  // the result is shown for.
  run(CAL_SPINUP_MS + CAL_SAMPLE_MS - 6000);
  CHECK(gMode == Mode::CalResult);
  CHECK(gCalOk);
  CHECK_NEAR(gCalResultRpm, 1836, 3);
  CHECK(gCalibrated);
  CHECK_NEAR(gMaxRpm, 1836, 3);
  CHECK(oled.last.hasText("CAL SAVED"));
  checkFrame("screen_cal_saved");
  {
    fan::CalRecord r;
    EEPROM.get(0, r);
    CHECK(fan::calRecordValid(r));
    CHECK_EQ(r.maxRpm, gMaxRpm);
  }
  CHECK_NEAR(fanLineDuty(), 700, 1);  // result screen: back to the setting
  run(kCalResultShowMs + 200);
  CHECK(gMode == Mode::Normal);
  CHECK_NEAR(fanLineDuty(), 700, 1);  // back to the user's setting
  sim::tachHz = 61.2 * 0.7;
  run(3000);
  CHECK(oled.last.hasText("70%"));
  CHECK(!oled.last.hasText("Hold knob 3s"));  // calibrated now
  CHECK(oled.last.hasText("Turn knob: speed"));

  // --- Calibration can be cancelled with a fresh press ---------------------
  sim::hold(PIN_ENC_SW);
  run(CAL_HOLD_MS + 200);
  sim::release(PIN_ENC_SW);
  run(500);
  CHECK(gMode == Mode::Calibrating);
  sim::press(PIN_ENC_SW, 100);
  CHECK(gMode == Mode::Normal);
  CHECK(gFanOn);
  CHECK_NEAR(fanLineDuty(), 700, 1);

  // --- Long press while OFF does nothing -----------------------------------
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gFanOn);
  sim::hold(PIN_ENC_SW);
  run(CAL_HOLD_MS + 500);
  sim::release(PIN_ENC_SW);
  run(200);
  CHECK(gMode == Mode::Normal);
  CHECK(!gFanOn);
  CHECK(fanStopped());

  // --- Diagnostics page via CON, back via BAK ------------------------------
  sim::press(PIN_ENC_SW, 100);  // ON again
  sim::tachHz = 42.8;
  run(2000);
  sim::press(PIN_KEY_CON, 100);
  run(300);
  CHECK(gMode == Mode::Diagnostics);
  CHECK(oled.last.hasText("DIAG"));
  CHECK(oled.last.hasText("Hz"));
  CHECK(oled.last.hasText("Gate PWM"));
  checkFrame("screen_diagnostics");
  sim::press(PIN_ENC_SW, 100);  // knob still toggles the fan on this page
  CHECK(!gFanOn);
  sim::press(PIN_KEY_BAK, 100);
  CHECK(gMode == Mode::Normal);

  // --- Display cadence --------------------------------------------------------
  const unsigned before = oled.frames;
  run(10000);
  CHECK(oled.frames - before >= 95);  // ~10 frames per second
  CHECK_EQ(oled.outOfBounds, 0);
}

static void boot2() {
  // After a power cycle: OFF, default setting, calibration remembered.
  CHECK(fanStopped());
  CHECK(!gFanOn);
  CHECK_EQ(gSetting, SETTING_DEFAULT_PCT);
  CHECK(gCalibrated);
  CHECK_NEAR(gMaxRpm, 1836, 3);
  run(1000);
  CHECK(fanStopped());
  CHECK(oled.last.hasText("FAN OFF"));
  CHECK_EQ(oled.outOfBounds, 0);
}

static void diagboot() {
  // Knob held while powering up: diagnostics page, and that press neither
  // toggles the fan nor starts a calibration.
  CHECK(gMode == Mode::Diagnostics);
  run(CAL_HOLD_MS + 1000);
  sim::release(PIN_ENC_SW);
  run(300);
  CHECK(!gFanOn);
  CHECK(fanStopped());
  CHECK(gMode == Mode::Diagnostics);
  sim::press(PIN_ENC_SW, 100);  // the next press works
  CHECK(gFanOn);
}

static void wrap() {
  // Powered up 10 s before millis() wraps (49.7 days of uptime in practice;
  // micros() wraps 1000x as often). Everything must keep working across it.
  CHECK(fanStopped());
  sim::press(PIN_ENC_SW, 100);
  CHECK(gFanOn);
  sim::tachHz = 50;
  run(4000);
  CHECK_NEAR(gRpm, 1500, 3);
  run(12000);  // crosses the wrap
  CHECK(millis() < 10000u);
  CHECK_NEAR(gRpm, 1500, 3);
  CHECK(!oled.last.hasText("NO TACH"));
  sim::turn(-10, 60);
  CHECK_EQ(gSetting, 40);
  CHECK_NEAR(fanLineDuty(), 400, 1);
  sim::press(PIN_ENC_SW, 100);
  CHECK(!gFanOn);
  CHECK(fanStopped());
  const unsigned before = oled.frames;
  run(2000);
  CHECK(oled.frames - before >= 19);  // display still refreshing
}

static void calfail() {
  sim::press(PIN_ENC_SW, 100);
  CHECK(gFanOn);
  sim::hold(PIN_ENC_SW);
  run(CAL_HOLD_MS + 200);
  sim::release(PIN_ENC_SW);
  CHECK(gMode == Mode::Calibrating);
  run(CAL_SPINUP_MS + CAL_SAMPLE_MS + 200);  // no tach at all
  CHECK(gMode == Mode::CalResult);
  CHECK(!gCalOk);
  CHECK(!gCalibrated);
  CHECK_EQ(gMaxRpm, TACH_DEFAULT_MAX_RPM);
  CHECK_EQ(EEPROM.writes, 0);
  CHECK(oled.last.hasText("CAL FAILED"));
  checkFrame("screen_cal_failed");
  sim::press(PIN_KEY_BAK, 100);
  CHECK(gMode == Mode::Normal);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s boot1|boot2|diagboot|wrap|calfail <eeprom-file> [shot-dir]\n", argv[0]);
    return 2;
  }
  const char *scenario = argv[1];
  const char *eeprom = argv[2];
  g_shotDir = argc > 3 ? argv[3] : nullptr;

  if (!strcmp(scenario, "boot1")) {
    remove(eeprom);
    powerUp(eeprom);
    boot1();
    CHECK(sim::eepromSave(eeprom));
  } else if (!strcmp(scenario, "boot2")) {
    powerUp(eeprom);
    boot2();
  } else if (!strcmp(scenario, "diagboot")) {
    sim::reset();
    sim::hold(PIN_ENC_SW);
    sim::eepromLoad(eeprom);
    setup();
    diagboot();
  } else if (!strcmp(scenario, "wrap")) {
    sim::reset();
    sim::nowUs = (uint64_t)0xFFFFFFFFu * 1000u - 10000000u;  // 10 s before millis() wraps
    setup();
    wrap();
  } else if (!strcmp(scenario, "calfail")) {
    remove(eeprom);
    powerUp(eeprom);
    calfail();
  } else {
    fprintf(stderr, "unknown scenario %s\n", scenario);
    return 2;
  }
  printf("(%u frames layout-checked)\n", g_framesChecked);
  return summary(scenario);
}
