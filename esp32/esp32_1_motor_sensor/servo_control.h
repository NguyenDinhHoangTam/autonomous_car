#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

class ServoControl {
public:
    ServoControl(int pin, int channel);   // ← thêm channel vào constructor
    void setAngle(int angle);

private:
    Servo servo;
    int pin;
    int channel;
};

#endif
