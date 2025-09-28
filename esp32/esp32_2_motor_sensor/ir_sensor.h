#pragma once
#include <Arduino.h>

#define IR_RIGHT_PIN 33

void setupIRSensor();
bool readIRRight();   // trả về true nếu thấy line (tuỳ module: HIGH/LOW)
