#include "servo_control.h"

ServoControl::ServoControl(int p, int ch) {
    pin = p;
    channel = ch;

    // Gắn chân với kênh PWM rõ ràng, xung 0.5ms - 2.5ms
    servo.attach(pin, 500, 2500, channel);
    servo.write(90);  // quay về giữa khi khởi tạo
}

void ServoControl::setAngle(int angle) {
    angle = constrain(angle, 0, 180);  // đảm bảo an toàn
    servo.write(angle);
}
