#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "settings.h"
#include "sensors.h"
#include "motor.h"
#include "motion.h"
#include "boot_button.h"
#include "leds.h"

// ---------------------------------------------------------------------------
// STARTUP
// ---------------------------------------------------------------------------

// Block until the stone is flat and still, then flash green to confirm ready.
void waitStill() {
  Serial.println("place flat and still...");
  bool isStill = false;
  while (!isStill) {
    ledWaitStill();

    int16_t samples[10];
    long sumX = 0;
    for (int i = 0; i < 10; i++) {
      samples[i] = readAX();
      sumX += samples[i];
      delay(20);
    }
    int16_t meanX = sumX / 10;
    int maxDev = 0;
    for (int i = 0; i < 10; i++) {
      int dev = abs(samples[i] - meanX);
      if (dev > maxDev) maxDev = dev;
    }
    if (maxDev < 500) isStill = true;
  }

  pixels.fill(pixels.Color(0, 255, 0));
  pixels.setBrightness(60);
  pixels.show();
  delay(800);
  clearLED();
  Serial.println("ready");
}

// Sit dormant at ~130µA until the boot button is held for LONG_PRESS_TIME.
// The ESP32 enters light sleep and wakes only when GPIO9 goes LOW, so the
// CPU is not running between presses. On wake, a genuine 2s hold is required
// before the green flash and release are accepted - spurious/short presses
// just send the device back to sleep.
void waitForStartup() {
  clearLED();
  motorOff();
  Serial.println("dormant - hold boot button 2s to start");
  Serial.flush();

  // Lock motor pin LOW before sleeping so the LEDC peripheral pausing/resuming
  // on sleep/wake transitions cannot cause a motor glitch.
  gpio_hold_en((gpio_num_t)PIN_MOTOR);

  while (true) {
    // Sleep until GPIO9 goes LOW (button pressed)
    gpio_wakeup_enable((gpio_num_t)BOOT_BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();

    // Just woke - confirm button is actually held
    if (digitalRead(BOOT_BUTTON_PIN) == HIGH) continue;

    unsigned long pressStart = millis();
    bool greenShown = false;

    while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      if (!greenShown && millis() - pressStart >= LONG_PRESS_TIME) {
        greenShown = true;
        pixels.fill(pixels.Color(0, 255, 0));
        pixels.setBrightness(60);
        pixels.show();
      }
      delay(16);
    }

    if (greenShown) {
      // Release the hold so LEDC can drive the motor again during gameplay
      gpio_hold_dis((gpio_num_t)PIN_MOTOR);
      clearLED();
      return;
    }
    // Short press - loop back and sleep again
  }
}

// ---------------------------------------------------------------------------
// SHUTDOWN
// ---------------------------------------------------------------------------

// Called after ledBreathe() sets shutdownRequested.
// At this point the button has been held for exactly LONG_PRESS_TIME.
// Flash red, wait for release, then lock out for LONG_PRESS_TIME so the
// same held gesture cannot immediately register as a startup.
void runShutdown() {
  motorOff();

  // All LEDs red + haptic pulse to confirm shutdown
  pixels.fill(pixels.Color(255, 0, 0));
  pixels.setBrightness(255);
  pixels.show();
  motorBuzz(180);

  // Wait for button release - 5s timeout prevents getting stuck with motor on
  // if the pin ever reads LOW continuously (floating/noise on battery power)
  unsigned long releaseWait = millis();
  while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    if (millis() - releaseWait > 5000) break;
    delay(16);
  }
  motorOff();
  clearLED();

  // Lockout: ignore all input for LONG_PRESS_TIME so the release
  // cannot bleed into the next startup detection loop
  delay(LONG_PRESS_TIME);

  Serial.println("shutdown complete");
}

// ---------------------------------------------------------------------------
// ARDUINO ENTRY POINTS
// ---------------------------------------------------------------------------

void setup() {
  // Release any gpio_hold from a previous run - it persists across soft resets
  // on ESP32-C3, so without this the motor pin can be stuck on after a reset.
  gpio_hold_dis((gpio_num_t)PIN_MOTOR);

  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);

  // Attach LEDC before the serial delay and immediately lock the pin LOW.
  // Without this, ledcAttach briefly drives the pin HIGH before ledcWrite(0)
  // takes effect, causing a motor buzz on every cold boot.
  ledcAttach(PIN_MOTOR, MOTOR_FREQ, MOTOR_RESOLUTION);
  ledcWrite(PIN_MOTOR, 0);
  gpio_hold_en((gpio_num_t)PIN_MOTOR);

  Serial.begin(115200);
  delay(2000);

  gpio_hold_dis((gpio_num_t)PIN_MOTOR);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission(true);

  pixels.begin();
  pixels.setBrightness(40);

  initBootButton();
  randomSeed(analogRead(0));
  // Device stays dormant - loop() handles the startup wait
}

void loop() {
  waitForStartup();
  waitStill();

  // Keep playing new games until a manual shutdown - game over just restarts
  while (true) {
    int faults = 0;
    int score = 0;
    unsigned long timeLimit = TIME_MAX;

    Serial.println("--- new game ---");
    delay(1000);

    while (faults < MAX_LIVES) {
      int gesture = random(4);
      bool correct = false;

      Serial.print("gesture: "); Serial.print(gesture);
      Serial.print(" faults: "); Serial.print(faults);
      Serial.print(" score: "); Serial.println(score);

      switch (gesture) {
        case 0: correct = ledBreathe(LED_LEFT,   timeLimit); break;
        case 1: correct = ledBreathe(LED_RIGHT,  timeLimit); break;
        case 2: correct = ledBreathe(LED_TOP,    timeLimit); break;
        case 3: correct = ledBreathe(LED_BOTTOM, timeLimit); break;
      }

      // Button held 2s - shut down and go dormant
      if (shutdownRequested) {
        shutdownRequested = false;
        Serial.println("shutdown requested");
        runShutdown();
        return;  // loop() restarts → waitForStartup()
      }

      if (correct) {
        score++;
        if (score % 2 == 0) {
          timeLimit = max((unsigned long)TIME_MIN, (unsigned long)(timeLimit * 0.9));
        }
        Serial.print("correct! score: "); Serial.println(score);
      } else {
        faults++;
        Serial.print("fault: "); Serial.println(faults);
        if (faults >= MAX_LIVES) {
          ledGameOver();
          Serial.print("game over. score: "); Serial.println(score);
          delay(3000);
        }
      }
    }
    // Game over - loop back and start a fresh game automatically
  }
}
