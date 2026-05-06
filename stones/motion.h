#ifndef MOTION_H
#define MOTION_H

#include "settings.h"
#include "sensors.h"

// ============================================================================
// MOTION DETECTION & DIRECTION FUNCTIONS
// ============================================================================
// These functions detect how the stone is tilted and in which direction.
// Uses accelerometer readings from the MPU-6050 sensor.
//
// How it works:
// 1. Read raw acceleration values from X and Y axes
// 2. Compare current values against calibrated REST positions
// 3. Calculate a percentage (0-100%) showing how far tilted in that direction
// 4. Use the percentage to decide if tilt is strong enough (threshold check)
// ============================================================================

// Get tilt percentage in a specific direction
// Returns 0-100, where 100 is fully tilted in that direction
// Uses calibration values from settings.h to map raw sensor readings to percentages
float getDirectionPct(int direction) {
  float ax = readAX();
  float ay = readAY();

  switch(direction) {
    case LED_TOP:
      // Forward tilt (top away from you)
      // ax increases as you tilt forward
      return ax > AX_REST ? constrain((ax - AX_REST) / (AX_FORWARD - AX_REST) * 100, 0, 100) : 0;
      
    case LED_LEFT:
      // Left tilt (left side down)
      // ay increases as you tilt left
      return ay > AY_REST ? constrain((ay - AY_REST) / (AY_LEFT - AY_REST) * 100, 0, 100) : 0;
      
    case LED_RIGHT:
      // Right tilt (right side down)
      // ay decreases as you tilt right
      return ay < AY_REST ? constrain((AY_REST - ay) / (AY_REST - AY_RIGHT) * 100, 0, 100) : 0;
      
    case LED_BOTTOM:
      // Back tilt (bottom away from you)
      // ax decreases as you tilt backward
      return ax < AX_REST ? constrain((AX_REST - ax) / (AX_REST - AX_BACK) * 100, 0, 100) : 0;
  }
  return 0;
}

// Check if device is tilted enough in a specific direction
// Returns true if tilt percentage exceeds 60% threshold
bool checkTilt(int direction) {
  return getDirectionPct(direction) >= 60.0f;
}

#endif
