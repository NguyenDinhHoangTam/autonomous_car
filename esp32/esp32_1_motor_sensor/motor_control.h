#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Motor A (ENA, IN1, IN2)
#define ENA 5
#define IN1 18
#define IN2 19

// Motor B (ENB, IN3, IN4)
#define ENB 23
#define IN3 21
#define IN4 22

void setupMotors();
void moveForward(int speed);
void moveBackward(int speed);
void turnLeft(int speed);
void turnRight(int speed);
void stopMotors();

#endif
