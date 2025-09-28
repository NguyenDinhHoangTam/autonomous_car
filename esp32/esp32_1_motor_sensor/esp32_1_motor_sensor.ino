#include "motor_control.h"
#include "sensor_control.h"

const unsigned long STATUS_INTERVAL_MS = 250;

void setup() {
 
  Serial.begin(115200);           // USB debug
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // UART tới Pi hoặc ESP32-2

  setupMotors();
  setupSensorControl();

  Serial.println("ESP32-1 ready!");
}

void loop() {

  static unsigned long lastStatus = 0;
  unsigned long now = millis();

  if (now - lastStatus >= STATUS_INTERVAL_MS) {
    lastStatus = now;

    // Gửi trạng thái cảm biến định kỳ
    String payload = buildSensorStatusCSV();
    Serial.println(payload);   // debug về USB
    sendSensorStatus(Serial2); // gửi cho thiết bị đối tác
  }

  // Các xử lý điều khiển động cơ khác có thể bổ sung sau
}
