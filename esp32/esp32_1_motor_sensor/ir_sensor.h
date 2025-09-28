#ifndef IR_SENSOR_H
#define IR_SENSOR_H

class IRSensor {
public:
    IRSensor(int pin);
    bool isLineDetected();  // true nếu thấy line (thường màu đen)

private:
    int pin;
};

#endif
