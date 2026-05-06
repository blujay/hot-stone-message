#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "settings.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

int chaseOrder[] = {LED_TOP, LED_RIGHT, LED_BOTTOM, LED_LEFT};
int wheelR[] = {180, 0,   255, 255};
int wheelG[] = {255, 255, 160, 20};
int wheelB[] = {0,   200, 0,   147};

bool shutdownRequested = false;

int16_t readAX() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

int16_t readAY() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3D);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

int16_t readAZ() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3F);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

int16_t readGZ() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  return (Wire.read() << 8 | Wire.read());
}

bool isFlipped() {
  return readAZ() < 0;
}

void getColour(int led, int &r, int &g, int &b) {
  switch(led) {
    case LED_TOP:    r=180; g=255; b=0;   break; // lime green
    case LED_LEFT:   r=255; g=20;  b=147; break; // hot pink
    case LED_BOTTOM: r=255; g=160; b=0;   break; // amber
    case LED_RIGHT:  r=0;   g=255; b=200; break; // aqua
  }
}

void motorOff() {
  ledcWrite(PIN_MOTOR, 0);
}

void gameOver() {
  for (int i = 0; i < 3; i++) {
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(255, 0, 0));
    pixels.show();
    ledcWrite(PIN_MOTOR, 255);
    delay(400);
    pixels.clear();
    pixels.show();
    motorOff();
    delay(200);
  }
  motorOff();
  pixels.clear();
  pixels.show();
}

void shutdownSequence() {
  motorOff();
  pixels.clear();
  pixels.show();

  // Flash red to confirm shutdown
  for (int i = 0; i < 3; i++) {
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(255, 0, 0));
    pixels.show();
    ledcWrite(PIN_MOTOR, 180);
    delay(300);
    pixels.clear();
    pixels.show();
    motorOff();
    delay(150);
  }
  motorOff();

  // Wait until the stone is returned right-side up before allowing startup.
  // Without this, the same held gesture that triggered shutdown immediately
  // registers as a startup signal.
  while (isFlipped()) {
    delay(50);
  }
  delay(500); // debounce after flip
}

// same as calibrate script - just percentage in correct direction
float getDirectionPct(int direction) {
  float ax = readAX();
  float ay = readAY();

  switch(direction) {
    case LED_TOP:
      return ax > AX_REST ? constrain((ax - AX_REST) / (AX_FORWARD - AX_REST) * 100, 0, 100) : 0;
    case LED_LEFT:
      return ay > AY_REST ? constrain((ay - AY_REST) / (AY_LEFT - AY_REST) * 100, 0, 100) : 0;
    case LED_RIGHT:
      return ay < AY_REST ? constrain((AY_REST - ay) / (AY_REST - AY_RIGHT) * 100, 0, 100) : 0;
    case LED_BOTTOM:
      return ax < AX_REST ? constrain((AX_REST - ax) / (AX_REST - AX_BACK) * 100, 0, 100) : 0;
  }
  return 0;
}

// simple threshold check - same as old simon says
bool checkTilt(int direction) {
  return getDirectionPct(direction) >= 60.0f;
}

bool breatheInstruct(int led, unsigned long timeLimit) {
  int r, g, b;
  getColour(led, r, g, b);
  unsigned long start = millis();
  float period = timeLimit * 0.8;
  unsigned long flipStart = 0;

  while (millis() - start < timeLimit) {

    // Check for shutdown gesture: hold flipped for FLIP_DURATION ms
    if (isFlipped()) {
      if (flipStart == 0) flipStart = millis();
      if (millis() - flipStart >= FLIP_DURATION) {
        shutdownRequested = true;
        motorOff();
        pixels.clear();
        pixels.show();
        return false;
      }
    } else {
      flipStart = 0;
    }

    if (checkTilt(led)) {
      // smooth 1 second sine ramp - plays fully, nothing interrupts
      for (int i = 0; i < 60; i++) {
        float phase = (float)i / 60.0f;
        float s = sin(phase * PI); // half sine - ramps up then down
        int buzzPower = (int)(s * 180);
        ledcWrite(PIN_MOTOR, buzzPower);
        pixels.clear();
        pixels.setPixelColor(led, pixels.Color(r, g, b));
        pixels.setBrightness(255);
        pixels.show();
        delay(16); // 60 * 16ms = ~1 second
      }
      motorOff();
      pixels.clear();
      pixels.show();
      delay(300);
      return true;
    }

    // gentle sine pulse
    unsigned long t = millis() - start;
    float phase = (float)(t % (unsigned long)period) / period;
    float s = (float)sin(phase * TWO_PI);
    float br = 8.0f + max(0.0f, s * 22.0f);

    pixels.clear();
    pixels.setPixelColor(led, pixels.Color(
      (int)(r * br / 30.0f),
      (int)(g * br / 30.0f),
      (int)(b * br / 30.0f)
    ));
    pixels.setBrightness(255);
    pixels.show();
    delay(16);
  }

  motorOff();
  pixels.clear();
  pixels.show();
  return false;
}

void waitStill() {
  Serial.println("place flat and still...");
  bool isStill = false;
  while (!isStill) {
    for (int br = 0; br <= 40; br++) {
      pixels.fill(pixels.Color(255, 255, 255));
      pixels.setBrightness(br);
      pixels.show();
      delay(8);
    }
    for (int br = 40; br >= 0; br--) {
      pixels.fill(pixels.Color(255, 255, 255));
      pixels.setBrightness(br);
      pixels.show();
      delay(8);
    }
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
    if (maxDev < 500) {
      isStill = true;
    }
  }
  pixels.fill(pixels.Color(0, 255, 0));
  pixels.setBrightness(60);
  pixels.show();
  delay(800);
  pixels.clear();
  pixels.show();
  Serial.println("ready");
}

void setup() {
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);

  Serial.begin(115200);
  delay(2000);

  ledcAttach(PIN_MOTOR, MOTOR_FREQ, MOTOR_RESOLUTION);
  ledcWrite(PIN_MOTOR, 0);

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

  waitStill();
  randomSeed(analogRead(0));
}

void loop() {
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

    switch(gesture) {
      case 0: correct = breatheInstruct(LED_LEFT,   timeLimit); break;
      case 1: correct = breatheInstruct(LED_RIGHT,  timeLimit); break;
      case 2: correct = breatheInstruct(LED_TOP,    timeLimit); break;
      case 3: correct = breatheInstruct(LED_BOTTOM, timeLimit); break;
    }

    if (shutdownRequested) {
      shutdownRequested = false;
      Serial.println("shutdown requested - interrupted game");
      shutdownSequence();
      waitStill();
      return;
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
        gameOver();
        Serial.print("Game over. Score: "); Serial.println(score);
        delay(3000);
      }
    }
  }
}