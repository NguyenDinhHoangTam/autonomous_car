#pragma once
#include <Arduino.h>

// L298N #2 - mapping theo bảng của bạn
// Motor A = bánh trước trái
#define ENA_PIN   19
#define IN1_PIN   18
#define IN2_PIN    5
// Motor B = bánh trước phải
#define ENB_PIN   13
#define IN3_PIN    4
#define IN4_PIN   14

// LEDC (PWM) cho ESP32
#define PWM_FREQ      20000     // 20 kHz để êm tiếng
#define PWM_RES_BITS  8         // 8-bit (0..255)
#define PWM_CH_A      0
#define PWM_CH_B      1

void setupMotors();

// Các hành vi cơ bản
void stopMotors();
void moveForward(int speed);    // 0..255
void moveBackward(int speed);
void turnLeft(int speed);       // quay trái tại chỗ
void turnRight(int speed);

// Hàm đặt speed riêng từng bánh (trái, phải)
void setMotorSpeeds(int leftSpeed, int rightSpeed);

// Hàm nội bộ: đặt hướng + tốc độ cho từng motor
void motorA(bool forward, int speed);
void motorB(bool forward, int speed);
