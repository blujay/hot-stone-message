#ifndef SETTINGS_H
#define SETTINGS_H

// pins
#define PIN_NEOPIXEL  3
#define PIN_MOTOR     2
#define PIN_SDA       10
#define PIN_SCL       8
#define BOOT_BUTTON_PIN 9

// motor
#define MOTOR_FREQ       5000
#define MOTOR_RESOLUTION 8

// neopixel
#define NUMPIXELS 4

// i2c
#define MPU_ADDR 0x68

// raw rest position - captured flat and still
#define AX_REST      -713.0f
#define AY_REST      1284.0f
#define AZ_REST      14877.0f

// raw full tilt values - captured at ~90 degrees
#define AX_FORWARD   16000.0f
#define AX_BACK     -15000.0f
#define AY_LEFT      16000.0f
#define AY_RIGHT    -9100.0f
#define AZ_FLIP     -18300.0f

// spin
#define GZ_SPIN_THRESHOLD 1000
#define SPIN_DEGREES      90

// flip hold
#define FLIP_DURATION 300

// game settings
#define TIME_MAX      4000
#define TIME_MIN      800
#define MAX_LIVES     3
#define HOLD_MAX      2000
#define HOLD_MIN      200
#define PURR_DURATION 800

// led positions
#define LED_TOP    0
#define LED_LEFT   1
#define LED_BOTTOM 2
#define LED_RIGHT  3

#endif