#include "motor_control.h"

void setup() {
  Serial.begin(115200);
  setupMotors();
  Serial.println("ESP32-1 ready!");
}

void loop() {
  moveForward(150);
  delay(1000);
  turnLeft(150);
  delay(1000);
  moveBackward(150);
  delay(1000);
  stopMotors();
  delay(1000);
}
