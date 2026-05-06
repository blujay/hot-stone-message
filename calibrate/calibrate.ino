#include <Wire.h>
#include "../stones/settings.h"
#include "../stones/sensors.h"



void printMenu() {
  Serial.println();
  Serial.println("=== STONE CALIBRATION ===");
  Serial.println("1 - REST    (place flat and still)");
  Serial.println("2 - FORWARD (tilt top away from you)");
  Serial.println("3 - BACK    (tilt bottom away from you)");
  Serial.println("4 - LEFT    (tilt left side down)");
  Serial.println("5 - RIGHT   (tilt right side down)");
  Serial.println("6 - FLIP    (turn face down)");
  Serial.println("7 - SPIN    (spin on flat surface)");
  Serial.println("=========================");
  Serial.println("hold position then press number + enter:");
}

void capture(String label, String axis, float value) {
  Serial.println();
  Serial.print(">>> "); Serial.print(label); Serial.print(" captured");
  Serial.print(" | "); Serial.print(axis); Serial.print(": "); Serial.println(value);
  Serial.println();
  Serial.println("note this value then update settings.h:");
  
  // print what to put in settings.h
  if (label == "REST") {
    Serial.print("  #define AX_REST "); Serial.println(avgAX());
    Serial.print("  #define AY_REST "); Serial.println(avgAY());
    Serial.print("  #define AZ_REST "); Serial.println(avgAZ());
    Serial.println("  dead zone = REST value +/- 25% of full range");
  }
}



void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  
  Serial.begin(115200);
  delay(3000);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission(true);
  Serial.println("MPU-6050 ready");
  printMenu();
}


void loop() {
  float ax = readAX();
  float ay = readAY();
  float az = readAZ();

  // calculate all percentages
  float forwardPct = ax > AX_REST ? constrain((ax - AX_REST) / (AX_FORWARD - AX_REST) * 100, 0, 100) : 0;
  float backPct    = ax < AX_REST ? constrain((AX_REST - ax) / (AX_REST - AX_BACK)    * 100, 0, 100) : 0;
  float leftPct    = ay > AY_REST ? constrain((ay - AY_REST) / (AY_LEFT - AY_REST)     * 100, 0, 100) : 0;
  float rightPct   = ay < AY_REST ? constrain((AY_REST - ay) / (AY_REST - AY_RIGHT)    * 100, 0, 100) : 0;
  float flipPct    = az < AZ_REST ? constrain((AZ_REST - az) / (AZ_REST - AZ_FLIP)     * 100, 0, 100) : 0;

  // find dominant direction
  float maxPct = max({forwardPct, backPct, leftPct, rightPct, flipPct});

  if (maxPct < 5) {
    Serial.println("REST - neutral");
    delay(200);
    return;
  }

  String direction = "";
  float pct = maxPct;

  if      (maxPct == forwardPct) direction = "FORWARD";
  else if (maxPct == backPct)    direction = "BACK";
  else if (maxPct == leftPct)    direction = "LEFT";
  else if (maxPct == rightPct)   direction = "RIGHT";
  else if (maxPct == flipPct)    direction = "FLIP";

  Serial.print(direction);
  Serial.print(": ");
  Serial.print((int)pct);
  Serial.print("%");

  if      (pct < 25) Serial.println(" - rest zone");
  else if (pct < 75) Serial.println(" - moving");
  else               Serial.println(" - SUCCESS");

  delay(200);
}