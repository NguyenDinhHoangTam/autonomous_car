// ESP32-A: Làm WiFi Client → kết nối ESP-B
#include <WiFi.h>

const char* ssid = "ESP_NET";
const char* password = "12345678";

const char* host = "192.168.4.1";  // IP mặc định của ESP SoftAP
const uint16_t port = 8888;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected to ESP-B! My IP: " + WiFi.localIP().toString());
}

void loop() {
  WiFiClient client;
  if (client.connect(host, port)) {
    String msg = "Hello from ESP-A\n";
    client.print(msg);
    Serial.print("Sent: "); Serial.print(msg);

    String reply = client.readStringUntil('\n');
    Serial.println("Reply: " + reply);
    client.stop();
  } else {
    Serial.println("Connection failed");
  }
  delay(2000);
}