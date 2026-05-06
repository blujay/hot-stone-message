#ifndef LEDS_H
#define LEDS_H

#include <Adafruit_NeoPixel.h>
#include "settings.h"
#include "motion.h"
#include "motor.h"
#include "boot_button.h"

// ============================================================================
// NEOPIXEL LED CONTROL FUNCTIONS
// ============================================================================
// These functions control the 4 WS2812B RGB LEDs (NeoPixels).
// Each LED corresponds to a direction: TOP, LEFT, BOTTOM, RIGHT
//
// Color scheme:
// - TOP (0):    Lime green (180, 255, 0)
// - LEFT (1):   Hot pink (255, 20, 147)
// - BOTTOM (2): Amber (255, 160, 0)
// - RIGHT (3):  Aqua (0, 255, 200)
// ============================================================================

// Global NeoPixel object
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Get the RGB color for a specific LED position
// Fills r, g, b references with the color values for that LED
void getColour(int led, int &r, int &g, int &b) {
  switch(led) {
    case LED_TOP:
      r = 180;
      g = 255;
      b = 0;
      break; // Lime green
      
    case LED_LEFT:
      r = 255;
      g = 20;
      b = 147;
      break; // Hot pink
      
    case LED_BOTTOM:
      r = 255;
      g = 160;
      b = 0;
      break; // Amber
      
    case LED_RIGHT:
      r = 0;
      g = 255;
      b = 200;
      break; // Aqua
  }
}

// Turn on a single LED with its assigned color at full brightness
void setLED(int led) {
  int r, g, b;
  getColour(led, r, g, b);
  pixels.clear();
  pixels.setPixelColor(led, pixels.Color(r, g, b));
  pixels.setBrightness(255);
  pixels.show();
}

// Turn off all LEDs
void clearLED() {
  pixels.clear();
  pixels.show();
}

// Gently pulse an LED with a breathing animation for a time limit
// If the direction is tilted strongly (checkTilt), performs a full ramp and returns true
// Otherwise, animates breathing until time runs out and returns false
// This is used in the game to both give instructions and detect player input
// Will exit immediately if boot button is pressed during animation
bool ledBreathe(int led, unsigned long timeLimit) {
  int r, g, b;
  getColour(led, r, g, b);
  
  unsigned long start = millis();
  float period = timeLimit * 0.8;  // Breathing cycle time

  while (millis() - start < timeLimit) {
    
    // Check if boot button pressed - interrupt animation immediately
    if (checkBootButtonLongPress()) {
      motorOff();
      clearLED();
      return false;  // Exit early
    }
    
    // Check if player tilted in the correct direction
    // If so, do a full bright ramp and return success
    if (checkTilt(led)) {
      // Smooth 1-second sine wave ramp up then down
      for (int i = 0; i < 60; i++) {
        float phase = (float)i / 60.0f;
        float s = sin(phase * PI);  // Half sine: goes from 0 to 1 to 0
        int buzzPower = (int)(s * 180);
        
        motorBuzz(buzzPower);  // Motor pulses in sync with brightness
        pixels.clear();
        pixels.setPixelColor(led, pixels.Color(r, g, b));
        pixels.setBrightness(255);
        pixels.show();
        delay(16);  // 60 frames * 16ms ≈ 1 second
        
        // Check for button interrupt during ramp
        if (checkBootButtonLongPress()) {
          motorOff();
          clearLED();
          return false;
        }
      }
      motorOff();
      clearLED();
      delay(300);
      return true;  // Success!
    }

    // Not tilted yet - gentle sine pulse to guide the player
    unsigned long elapsed = millis() - start;
    float phase = (float)(elapsed % (unsigned long)period) / period;
    float s = sin(phase * TWO_PI);  // Full sine wave: smooth cycle
    float brightness = 8.0f + max(0.0f, s * 22.0f);  // Brightness ranges 8-30

    pixels.clear();
    pixels.setPixelColor(led, pixels.Color(
      (int)(r * brightness / 30.0f),
      (int)(g * brightness / 30.0f),
      (int)(b * brightness / 30.0f)
    ));
    pixels.setBrightness(255);
    pixels.show();
    delay(16);
  }

  // Time ran out without successful tilt
  motorOff();
  clearLED();
  return false;
}

// Flash animation: pulse red quickly 3 times when player loses
// Synchronized motor pulses create an alarming haptic effect
// Used to signal game over with clear, alarming feedback
void ledGameOver() {
  for (int i = 0; i < 3; i++) {
    // Red flash on + motor pulse
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(255, 0, 0));  // Bright red
    pixels.show();
    motorBuzz(255);  // Full power buzz during flash
    delay(400);
    
    // All off
    clearLED();
    motorOff();
    delay(200);
  }
  motorOff();
  clearLED();
}

// Breathing white animation while waiting for player to place stone flat
// Gently fades brightness up and down to signal "waiting, do not move"
void ledWaitStill() {
  // Fade in: brightness 0 to 40
  for (int brightness = 0; brightness <= 40; brightness++) {
    pixels.fill(pixels.Color(255, 255, 255));  // White
    pixels.setBrightness(brightness);
    pixels.show();
    delay(8);
  }
  
  // Fade out: brightness 40 to 0
  for (int brightness = 40; brightness >= 0; brightness--) {
    pixels.fill(pixels.Color(255, 255, 255));  // White
    pixels.setBrightness(brightness);
    pixels.show();
    delay(8);
  }
}

#endif
