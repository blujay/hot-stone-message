#ifndef SETTINGS_H
#define SETTINGS_H

// pins
#define PIN_NEOPIXEL  3
#define PIN_MOTOR     2
#define PIN_SDA       10
#define PIN_SCL       8

// motor
#define MOTOR_FREQ       5000
#define MOTOR_RESOLUTION 8

// neopixel
#define NUMPIXELS 4

// i2c
#define MPU_ADDR 0x68

// rest position
#define AX_REST      -713.0
#define AY_REST      1284.0
#define AZ_REST      14877.0

// full tilt values
#define AX_FORWARD   16000.0
#define AX_BACK     -15000.0
#define AY_LEFT      16000.0
#define AY_RIGHT     -9100.0
#define AZ_FLIP     -18300.0

// thresholds - 75% of full range from rest
#define TILT_FORWARD  ((AX_REST + (AX_FORWARD - AX_REST) * 0.75))
#define TILT_BACK     ((AX_REST + (AX_BACK    - AX_REST) * 0.75))
#define TILT_LEFT     ((AY_REST + (AY_LEFT    - AY_REST) * 0.75))
#define TILT_RIGHT    ((AY_REST + (AY_RIGHT   - AY_REST) * 0.75))
#define TILT_FLIP     ((AZ_REST + (AZ_FLIP    - AZ_REST) * 0.75))

// dead zone - 25% of full range from rest
#define DEAD_FORWARD  ((AX_REST + (AX_FORWARD - AX_REST) * 0.25))
#define DEAD_BACK     ((AX_REST + (AX_BACK    - AX_REST) * 0.25))
#define DEAD_LEFT     ((AY_REST + (AY_LEFT    - AY_REST) * 0.25))
#define DEAD_RIGHT    ((AY_REST + (AY_RIGHT   - AY_REST) * 0.25))

// spin
#define GZ_SPIN_THRESHOLD 1000
#define SPIN_DEGREES       90

// flip hold duration
#define FLIP_DURATION 300

// game settings
#define TIME_MAX      4000
#define TIME_MIN      800
#define MAX_LIVES     3
#define PURR_DURATION 800

// led positions
#define LED_TOP    0
#define LED_LEFT   1
#define LED_BOTTOM 2
#define LED_RIGHT  3

#endif