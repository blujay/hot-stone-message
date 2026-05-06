#ifndef BOOT_BUTTON_H
#define BOOT_BUTTON_H

#include "settings.h"

// ============================================================================
// BOOT BUTTON CONTROL
// ============================================================================
// Detects long-press on the ESP32-C3 boot button (GPIO 9)
// Used for startup and shutdown of the game
//
//
// Button behavior:
// - Long-press (> 1.5 seconds) in OFF state: starts the game
// - Long-press (> 1.5 seconds) in RUNNING state: stops and enters deep sleep
// - Short presses are ignored (debounced)
// ============================================================================

// Button states
#define BUTTON_IDLE    0
#define BUTTON_PRESSED 1

// Debounce threshold (milliseconds) - ignore bounces shorter than this
#define DEBOUNCE_TIME 50

// Long-press threshold (milliseconds) - must hold for this long to trigger
#define LONG_PRESS_TIME 2000

// Track button state and timing
unsigned long buttonPressStart = 0;
int lastButtonState = HIGH;  // Boot button is pulled high, active low
bool buttonThresholdFlashed = false;  // Track if we've given feedback for this press
bool shutdownRequested = false;

// Initialize boot button for reading
// Must be called in setup()
void initBootButton() {
  pinMode(BOOT_BUTTON_PIN, INPUT);
}

// Reset button state machine - call before entering a new wait loop
// so stale state from a previous press doesn't carry over
void resetButtonState() {
  buttonPressStart = 0;
  lastButtonState = HIGH;
  buttonThresholdFlashed = false;
}

// Check if boot button has been long-pressed
// Returns true once per long-press event
// Call this regularly in your loop
bool checkBootButtonLongPress() {
  int buttonState = digitalRead(BOOT_BUTTON_PIN);
  
  // Button pressed (active low on boot button)
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(DEBOUNCE_TIME);  // Debounce
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      buttonPressStart = millis();
      buttonThresholdFlashed = false;  // Reset feedback flag for new press
    }
    lastButtonState = LOW;
  }
  
  // Button released
  else if (buttonState == HIGH && lastButtonState == LOW) {
    delay(DEBOUNCE_TIME);  // Debounce
    if (digitalRead(BOOT_BUTTON_PIN) == HIGH) {
      // Check if hold time exceeded long-press threshold
      unsigned long holdTime = millis() - buttonPressStart;
      lastButtonState = HIGH;
      buttonThresholdFlashed = false;  // Reset feedback flag on release
      
      if (holdTime >= LONG_PRESS_TIME) {
        return true;  // Long-press detected!
      }
    }
  }
  
  lastButtonState = buttonState;
  return false;
}

// Check if boot button has reached the long-press threshold while still holding
// Returns true once per press when 2-second mark is reached
// Used to give visual feedback that user can now release the button
bool checkBootButtonThresholdReached() {
  int buttonState = digitalRead(BOOT_BUTTON_PIN);
  
  // Button is currently pressed, threshold reached, and we haven't flashed yet for this press
  if (buttonState == LOW && !buttonThresholdFlashed) {
    unsigned long holdTime = millis() - buttonPressStart;
    
    if (holdTime >= LONG_PRESS_TIME) {
      buttonThresholdFlashed = true;  // Mark that we've given feedback for this press
      return true;  // Signal that threshold reached!
    }
  }
  
  return false;
}

#endif
