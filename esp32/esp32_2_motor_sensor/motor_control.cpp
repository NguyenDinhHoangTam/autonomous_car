#include "motor_control.h"

static inline int clamp255(int v) {
  if (v < 0) v = 0;
  if (v > 255) v = 255;
  return v;
}

void setupMotors() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  // cấu hình PWM
  ledcSetup(PWM_CH_A, PWM_FREQ, PWM_RES_BITS);
  ledcSetup(PWM_CH_B, PWM_FREQ, PWM_RES_BITS);
  ledcAttachPin(ENA_PIN, PWM_CH_A);
  ledcAttachPin(ENB_PIN, PWM_CH_B);

  stopMotors();
}

void motorA(bool forward, int speed) {
  speed = clamp255(speed);
  digitalWrite(IN1_PIN, forward ? HIGH : LOW);
  digitalWrite(IN2_PIN, forward ? LOW  : HIGH);
  ledcWrite(PWM_CH_A, speed);
}

void motorB(bool forward, int speed) {
  speed = clamp255(speed);
  digitalWrite(IN3_PIN, forward ? HIGH : LOW);
  digitalWrite(IN4_PIN, forward ? LOW  : HIGH);
  ledcWrite(PWM_CH_B, speed);
}

void stopMotors() {
  ledcWrite(PWM_CH_A, 0);
  ledcWrite(PWM_CH_B, 0);
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
  // Trái lùi, phải tiến
  motorA(false, speed);
  motorB(true,  speed);
}

void turnRight(int speed) {
  // Trái tiến, phải lùi
  motorA(true,  speed);
  motorB(false, speed);
}

void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  // leftSpeed/rightSpeed: -255..255 (âm = lùi)
  if (leftSpeed >= 0) motorA(true,  leftSpeed);
  else                motorA(false, -leftSpeed);

  if (rightSpeed >= 0) motorB(true,  rightSpeed);
  else                 motorB(false, -rightSpeed);
}
