#pragma once

#include <Arduino.h>
#include <Stream.h>

// Chân kết nối phần cứng cho ESP32-1
#define IR_LEFT_PIN 32
#define US_LEFT_TRIG_PIN 25
#define US_LEFT_ECHO_PIN 35
#define US_REAR_TRIG_PIN 26
#define US_REAR_ECHO_PIN 34
#define SERVO_REAR_PIN 27

// Khởi tạo toàn bộ cảm biến và servo phía sau
void setupSensorControl();

// Đọc trạng thái cảm biến line IR bên trái (true nếu phát hiện line)
bool readIRLeft();
// Đọc vị trí line đã chuẩn hóa trong khoảng 0..1 (0 = sát trái, 1 = sát phải)
float readLinePosition();

// Gửi gói tin LINE,<position> qua cổng Serial tương ứng
void sendLineReading(Stream &port);


// Đo khoảng cách cảm biến siêu âm bên trái (đơn vị cm)
long readUltrasonicLeft();

// Đo khoảng cách cảm biến siêu âm phía sau (đơn vị cm)
long readUltrasonicRear();

// Điều chỉnh servo quét phía sau (0..180 độ)
void setRearServoAngle(int angle);

// Lấy lại góc hiện tại của servo phía sau
int getRearServoAngle();

// Xây dựng chuỗi CSV theo giao thức STAT,<left_cm>,<rear_cm>,<ir_left>,<servo_deg>
String buildSensorStatusCSV();

// Gửi chuỗi trạng thái qua cổng Serial tương ứng
void sendSensorStatus(Stream &port);
