#include "ir_sensor.h"

IRSensor::IRSensor(int pin) : pin_(pin) {
  pinMode(pin_, INPUT);
}

bool IRSensor::isLineDetected() {
  // Các module cảm biến line phổ biến đưa chân OUT xuống LOW khi gặp line đen.
  return digitalRead(pin_) == LOW;
}

namespace {
  IRSensor g_rightIR(IR_RIGHT_PIN);
}

void setupIRSensor() {
  // Không cần làm gì thêm: constructor đã cấu hình pinMode.
}

bool readIRRight() {
  return g_rightIR.isLineDetected();
}
