#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include "settings.h"

// ============================================================================
// MPU-6050 ACCELEROMETER & GYROSCOPE SENSOR FUNCTIONS
// ============================================================================
// These functions read raw data from the MPU-6050 sensor connected via I2C.
// Each axis returns a 16-bit signed integer value.
// 
// Register explanation:
// - 0x3B: Accelerometer X (2 bytes)
// - 0x3D: Accelerometer Y (2 bytes)
// - 0x3F: Accelerometer Z (2 bytes)
// - 0x47: Gyroscope Z (2 bytes)
//
// The bit-shifting (read << 8 | read) combines two bytes into one 16-bit value:
// - First read is the high byte (shifted left 8 bits)
// - Second read is the low byte
// ============================================================================

// Read raw acceleration on X axis from MPU-6050
int16_t readAX() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // Register: Accel X High Byte
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

// Read raw acceleration on Y axis from MPU-6050
int16_t readAY() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3D);  // Register: Accel Y High Byte
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

// Read raw acceleration on Z axis from MPU-6050
int16_t readAZ() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3F);  // Register: Accel Z High Byte
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

// Read raw rotation on Z axis (gyroscope) from MPU-6050
int16_t readGZ() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47);  // Register: Gyro Z High Byte
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

// ============================================================================
// AVERAGING FUNCTIONS
// ============================================================================
// These functions take 20 readings with a small delay between each, then
// average them together. This removes noise and gives more stable values.
// Useful for calibration and getting accurate baseline measurements.
// ============================================================================

// Take 20 readings of acceleration on X axis and average them
float avgAX() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += readAX();
    delay(10);
  }
  return sum / 20.0;
}

// Take 20 readings of acceleration on Y axis and average them
float avgAY() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += readAY();
    delay(10);
  }
  return sum / 20.0;
}

// Take 20 readings of acceleration on Z axis and average them
float avgAZ() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += readAZ();
    delay(10);
  }
  return sum / 20.0;
}

// Take 20 readings of rotation on Z axis and average them
float avgGZ() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += readGZ();
    delay(10);
  }
  return sum / 20.0;
}

#endif
