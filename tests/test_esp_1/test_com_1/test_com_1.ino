// ESP32-B: Làm WiFi Server + SoftAP
#include <WiFi.h>

WiFiServer server(8888);

void setup() {
  Serial.begin(115200);
  // Tạo WiFi AP tên "ESP_NET", mật khẩu "12345678"
  WiFi.softAP("ESP_NET", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address: " + IP.toString());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected!");
    while (client.connected()) {
      if (client.available()) {
        String msg = client.readStringUntil('\n');
        Serial.println("Received: " + msg);
        client.println("ACK from B");  // Gửi phản hồi
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}
