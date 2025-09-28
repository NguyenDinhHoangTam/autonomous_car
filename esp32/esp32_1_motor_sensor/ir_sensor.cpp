#include "ir_sensor.h"
#include <Arduino.h>

IRSensor::IRSensor(int p) {
    pin = p;
    pinMode(pin, INPUT);
}

bool IRSensor::isLineDetected() {
    return digitalRead(pin) == LOW;  // giả sử line đen là LOW
}
