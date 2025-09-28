#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

class UltrasonicSensor {
public:
    UltrasonicSensor(int trigPin, int echoPin);
    long getDistance();  // trả về khoảng cách theo mm hoặc cm

private:
    int trigPin;
    int echoPin;
};

#endif
