#include "motor_control.h"
#include <Arduino.h>

static inline int clamp255(int v) {
  return constrain(v, 0, 255);
}

void setupMotors() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  // PWM (LEDC) – dùng high-level API
  ledcAttach(ENA_PIN, PWM_FREQ, PWM_RES_BITS); // PWM cho motor A
  ledcAttach(ENB_PIN, PWM_FREQ, PWM_RES_BITS); // PWM cho motor B

  stopMotors();
}

void motorA(bool forward, int speed) {
  speed = clamp255(speed);
  digitalWrite(IN1_PIN, forward ? HIGH : LOW);
  digitalWrite(IN2_PIN, forward ? LOW  : HIGH);
  ledcWrite(ENA_PIN, speed);
}

void motorB(bool forward, int speed) {
  speed = clamp255(speed);
  digitalWrite(IN3_PIN, forward ? HIGH : LOW);
  digitalWrite(IN4_PIN, forward ? LOW  : HIGH);
  ledcWrite(ENB_PIN, speed);
}

void stopMotors() {
  ledcWrite(ENA_PIN, 0);
  ledcWrite(ENB_PIN, 0);
}

void moveForward(int speed) {
  motorA(true,  speed);
  motorB(true,  speed);
}

void moveBackward(int speed) {
  motorA(false, speed);
  motorB(false, speed);
}

void turnLeft(int speed) {
  motorA(false, speed);
  motorB(true,  speed);
}

void turnRight(int speed) {
  motorA(true,  speed);
  motorB(false, speed);
}

void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  if (leftSpeed >= 0) motorA(true,  leftSpeed);
  else                motorA(false, -leftSpeed);

  if (rightSpeed >= 0) motorB(true,  rightSpeed);
  else                 motorB(false, -rightSpeed);
}
