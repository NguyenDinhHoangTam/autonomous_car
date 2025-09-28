#include "ultrasonic_sensor.h"
#include <Arduino.h>

UltrasonicSensor::UltrasonicSensor(int trig, int echo) {
    trigPin = trig;
    echoPin = echo;
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

long UltrasonicSensor::getDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms ~ 5m
    long distance = duration * 0.034 / 2;
    return distance;
}
