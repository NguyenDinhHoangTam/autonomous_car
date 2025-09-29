#include "servo_control.h"
#include <ESP32Servo.h>

static Servo servoFront;
static int currentAngle = 90;

void setupServo() {
  // Tuỳ servo, biên độ xung chuẩn ~ 500..2400us
  servoFront.attach(SERVO_FRONT_PIN, 500, 2400);
  setServoAngle(90); // trung tâm
}

void setServoAngle(int deg) {
  if (deg < 0) deg = 0;
  if (deg > 180) deg = 180;
  currentAngle = deg;
  servoFront.write(deg);
}

int getServoAngle() {
  return currentAngle;
}
