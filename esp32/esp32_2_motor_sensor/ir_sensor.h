#pragma once

#include <Arduino.h>

#define IR_RIGHT_PIN 33

class IRSensor {
public:
  explicit IRSensor(int pin);
  bool isLineDetected();

private:
  int pin_;
};

void setupIRSensor();
bool readIRRight();  // trả về true nếu thấy line (tuỳ module: HIGH/LOW)
