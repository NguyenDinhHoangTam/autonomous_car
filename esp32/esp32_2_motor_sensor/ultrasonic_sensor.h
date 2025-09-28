#pragma once
#include <Arduino.h>

// Cảm biến siêu âm (phải, trước) theo bảng của bạn
#define US_RIGHT_TRIG 25
#define US_RIGHT_ECHO 34   // input-only (nhớ level shifter)
#define US_FRONT_TRIG 32
#define US_FRONT_ECHO 36   // input-only (nhớ level shifter)

enum UltrasonicPos {
  SENSOR_RIGHT = 0,
  SENSOR_FRONT = 1
};

void setupUltrasonicSensors();

// Trả về khoảng cách đơn vị cm; -1 nếu timeout/không đọc được
int readUltrasonic(UltrasonicPos pos);
