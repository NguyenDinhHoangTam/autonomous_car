#include "sensor_control.h"

#include "ir_sensor.h"
#include "servo_control.h"
#include "ultrasonic_sensor.h"

namespace {
IRSensor g_leftIR(IR_LEFT_PIN);
UltrasonicSensor g_leftUltrasonic(US_LEFT_TRIG_PIN, US_LEFT_ECHO_PIN);
UltrasonicSensor g_rearUltrasonic(US_REAR_TRIG_PIN, US_REAR_ECHO_PIN);
ServoControl g_rearServo(SERVO_REAR_PIN);
int g_servoAngle = 90;
bool g_initialized = false;
}

void setupSensorControl() {
  g_rearServo.setAngle(g_servoAngle);
  g_initialized = true;
}

static void ensureInitialized() {
  if (!g_initialized) {
    setupSensorControl();
  }
}

bool readIRLeft() {
  ensureInitialized();
  return g_leftIR.isLineDetected();
}

long readUltrasonicLeft() {
  ensureInitialized();
  return g_leftUltrasonic.getDistance();
}

long readUltrasonicRear() {
  ensureInitialized();
  return g_rearUltrasonic.getDistance();
}

void setRearServoAngle(int angle) {
  ensureInitialized();
  g_servoAngle = constrain(angle, 0, 180);
  g_rearServo.setAngle(g_servoAngle);
}

int getRearServoAngle() {
  ensureInitialized();
  return g_servoAngle;
}

String buildSensorStatusCSV() {
  long distLeft = readUltrasonicLeft();
  long distRear = readUltrasonicRear();
  bool irLeft = readIRLeft();
  int servoDeg = getRearServoAngle();

  String payload = "STAT,";
  payload += distLeft;
  payload += ',';
  payload += distRear;
  payload += ',';
  payload += (irLeft ? 1 : 0);
  payload += ',';
  payload += servoDeg;
  return payload;
}

void sendSensorStatus(Stream &port) {
  String line = buildSensorStatusCSV();
  port.println(line);
}
