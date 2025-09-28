#include "servo_control.h"

ServoControl::ServoControl(int p) {
    pin = p;
    servo.attach(pin);
}

void ServoControl::setAngle(int angle) {
    angle = constrain(angle, 0, 180);
    servo.write(angle);
}
