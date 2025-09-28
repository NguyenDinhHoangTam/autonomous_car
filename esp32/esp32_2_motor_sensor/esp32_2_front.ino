#include "motor_control.h"
#include "ir_sensor.h"
#include "ultrasonic_sensor.h"
#include "servo_control.h"
#include "uart_comm.h"

void setup() {
  Serial.begin(115200);    // Debug
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // UART2 với Pi (RX=16, TX=17)

  setupMotors();
  setupIRSensor();
  setupUltrasonicSensors();
  setupServo();

  Serial.println("ESP32-2 Ready!");
}

void loop() {
  // Nhận lệnh từ Pi
  if (Serial2.available()) {
    char cmd = Serial2.read();

    if (cmd == 'F') moveForward(150);
    else if (cmd == 'B') moveBackward(150);
    else if (cmd == 'L') turnLeft(150);
    else if (cmd == 'R') turnRight(150);
    else if (cmd == 'S') stopMotors();
  }

  // Có thể gửi dữ liệu về Pi nếu muốn
  
  int distFront = readUltrasonic(SENSOR_FRONT);
  int distRight = readUltrasonic(SENSOR_RIGHT);
  bool irRight = readIRRight();
  uartSendStatus(distFront, distRight, irRight);
  delay(100);
}
