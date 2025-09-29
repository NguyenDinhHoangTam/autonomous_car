#include "ir_sensor.h"

namespace {

class IRSensor {
public:
  explicit IRSensor(uint8_t pin) : pin_(pin) {}

  void begin() {
    pinMode(pin_, INPUT);
  }

  bool isLineDetected() const {
    // Các module cảm biến line phổ biến đưa chân OUT xuống LOW khi gặp line đen.
    return digitalRead(pin_) == LOW;
  }

private:
  uint8_t pin_;
};

IRSensor g_rightIR(IR_RIGHT_PIN);
bool g_initialized = false;

void ensureInitialized() {
  if (!g_initialized) {
    g_rightIR.begin();
    g_initialized = true;
  }
}

}  // namespace

void setupIRSensor() {
  ensureInitialized();
}

bool readIRRight() {
  ensureInitialized();
  return g_rightIR.isLineDetected();
}
