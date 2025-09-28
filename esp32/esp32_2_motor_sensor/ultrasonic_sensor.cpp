#include "ultrasonic_sensor.h"

static long measureDistanceCM(uint8_t trigPin, uint8_t echoPin) {
  // Phát xung 10us
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  pinMode(echoPin, INPUT);
  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL); // timeout 30ms ~ 5m
  if (duration == 0) return -1; // timeout

  // Tốc độ âm thanh ~ 0.034 cm/us, chia 2 vì đi-về
  long distance = (long)(duration * 0.034 / 2.0);
  return distance;
}

void setupUltrasonicSensors() {
  pinMode(US_RIGHT_TRIG, OUTPUT);
  pinMode(US_FRONT_TRIG, OUTPUT);
  pinMode(US_RIGHT_ECHO, INPUT);
  pinMode(US_FRONT_ECHO, INPUT);
  digitalWrite(US_RIGHT_TRIG, LOW);
  digitalWrite(US_FRONT_TRIG, LOW);
}

int readUltrasonic(UltrasonicPos pos) {
  if (pos == SENSOR_RIGHT) return measureDistanceCM(US_RIGHT_TRIG, US_RIGHT_ECHO);
  else                     return measureDistanceCM(US_FRONT_TRIG, US_FRONT_ECHO);
}
