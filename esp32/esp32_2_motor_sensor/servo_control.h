#pragma once
#include <Arduino.h>

#define SERVO_FRONT_PIN 26

void setupServo();
void setServoAngle(int deg);    // 0..180
