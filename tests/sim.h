// Simulation helpers: drive time, pins, the encoder, buttons and the fan's
// tach signal around the real sketch (setup()/loop()) on the host.
#pragma once

#include <stdint.h>

#include "stubs/Arduino.h"

void setup();
void loop();
void isr_PCINT2_vect();

namespace sim {

// Pulse rate (Hz) the simulated fan produces on the tach input; 0 = stopped.
extern double tachHz;

void reset();                   // fresh "power-up" state for pins and registers
void advance(uint32_t ms);      // run loop() once per millisecond for `ms`
void turn(int detents, uint32_t msBetween);  // + = decoder-positive direction
void bounceDetent(int dir);     // one detent with contact bounce on every edge
void press(uint8_t pin, uint32_t holdMs);    // press, hold, release, settle
void hold(uint8_t pin);         // press and keep holding
void release(uint8_t pin);

// Timer1 state as the fan would see it on the gate of Q1.
bool pwmConnected();            // COM1A1 set: Timer1 drives D9
uint16_t pwmTop();
// Gate duty in permille (time D9 is HIGH), from the register state.
uint16_t gateDutyPermille();

bool eepromLoad(const char *path);
bool eepromSave(const char *path);

}  // namespace sim
