#ifndef MOTOR_H
#define MOTOR_H

#include "settings.h"

// ============================================================================
// MOTOR / HAPTIC FEEDBACK FUNCTIONS
// ============================================================================
// These functions control the vibration motor using PWM (Pulse Width Modulation).
// PWM lets us vary motor power from 0 (off) to 255 (full power) for different
// intensities of vibration feedback.
//
// The motor is attached to PIN_MOTOR and uses:
// - MOTOR_FREQ: PWM frequency (5000 Hz)
// - MOTOR_RESOLUTION: 8-bit resolution (values 0-255)
// ============================================================================

// Turn off the motor - set PWM output to 0
void motorOff() {
  ledcWrite(PIN_MOTOR, 0);
}

// Buzz motor at a specific power level
// power: 0 (off) to 255 (full power)
// Call motorOff() to stop
void motorBuzz(int power) {
  ledcWrite(PIN_MOTOR, constrain(power, 0, 255));
}

// Ramp motor power from start to end over a duration
// Smoothly increases or decreases motor intensity
// This creates a building or fading rumble effect
// 
// startPower: initial PWM value (0-255)
// endPower: final PWM value (0-255)
// durationMs: how long the ramp takes in milliseconds
void motorRamp(int startPower, int endPower, unsigned long durationMs) {
  int steps = 50;  // Number of ramp steps
  
  for (int step = 0; step <= steps; step++) {
    // Linear interpolation: blend from start to end power
    float progress = (float)step / (float)steps;  // 0.0 to 1.0
    int currentPower = (int)(startPower + (endPower - startPower) * progress);
    
    ledcWrite(PIN_MOTOR, currentPower);
    delay(durationMs / steps);
  }
}

// Single pulse of motor vibration
// Quick rumble with smooth ramp up and down
// 
// power: peak PWM value (0-255) at the middle of the pulse
// durationMs: how long the entire pulse takes (up + down)
void motorPulse(int power, unsigned long durationMs) {
  motorRamp(0, power, durationMs / 2);     // Ramp up to peak
  motorRamp(power, 0, durationMs / 2);     // Ramp down to off
}

#endif
