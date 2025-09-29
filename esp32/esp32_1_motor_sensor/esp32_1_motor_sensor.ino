#include "motor_control.h"
#include "sensor_control.h"
#include "servo_control.h"   


const unsigned long STATUS_INTERVAL_MS = 250;
const int DEFAULT_DRIVE_SPEED = 180;

namespace {
String g_usbRxBuffer;
String g_uartRxBuffer;

int normalizeSpeed(const String &token) {
  if (token.length() == 0) {
    return DEFAULT_DRIVE_SPEED;
  }
  int value = token.toInt();
  if (value <= 0) {
    value = DEFAULT_DRIVE_SPEED;
  }
  return constrain(value, 0, 255);
}

void executeDriveCommand(char command, int speed) {
  switch (command) {
    case 'F':
      moveForward(speed);
      break;
    case 'B':
      moveBackward(speed);
      break;
    case 'L':
      turnLeft(speed);
      break;
    case 'R':
      turnRight(speed);
      break;
    case 'S':
      stopMotors();
      break;
    default:
      Serial.print("Unknown drive command: ");
      Serial.println(command);
      break;
  }
}

void handleCommandPayload(const String &payload) {
  if (payload.length() == 0) {
    return;
  }

  String trimmed = payload;
  trimmed.trim();
  if (trimmed.length() == 0) {
    return;
  }

  int commaIndex = trimmed.indexOf(',');
  String commandToken = commaIndex >= 0 ? trimmed.substring(0, commaIndex)
                                        : trimmed;
  String speedToken = commaIndex >= 0 ? trimmed.substring(commaIndex + 1) : "";

  commandToken.trim();
  speedToken.trim();

  if (speedToken.length() == 0 && commandToken.length() > 1) {
    // Support compact forms like "F200".
    int digitIndex = 1;
    while (digitIndex < commandToken.length() &&
           isDigit(commandToken.charAt(digitIndex))) {
      digitIndex++;
    }
    if (digitIndex > 1) {
      speedToken = commandToken.substring(1, digitIndex);
      commandToken = commandToken.substring(0, 1);
    }
  }

  if (commandToken.length() == 0) {
    return;
  }

  char command = toupper(commandToken.charAt(0));
  int speed = normalizeSpeed(speedToken);
  executeDriveCommand(command, speed);
}

void processIncomingLine(const String &line) {
  if (line.length() == 0) {
    return;
  }

  String trimmed = line;
  trimmed.trim();
  if (trimmed.length() == 0) {
    return;
  }

  if (trimmed.startsWith("CMD")) {
    String payload = trimmed.substring(3);
    if (payload.startsWith(",")) {
      payload.remove(0, 1);
    }
    handleCommandPayload(payload);
  } else {
    // Accept bare one-letter commands for manual debugging (e.g. via serial monitor).
    handleCommandPayload(trimmed);
  }
}

void pollSerialPort(Stream &port, String &buffer) {
  while (port.available() > 0) {
    char c = static_cast<char>(port.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      processIncomingLine(buffer);
      buffer = "";
    } else if (buffer.length() < 64) {
      buffer += c;
    }
  }
}
} // namespace
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

  
  pollSerialPort(Serial, g_usbRxBuffer);
  pollSerialPort(Serial2, g_uartRxBuffer);


  if (now - lastStatus >= STATUS_INTERVAL_MS) {
    lastStatus = now;

    // Gửi trạng thái cảm biến định kỳ
    String payload = buildSensorStatusCSV();
    Serial.println(payload);   // debug về USB
    sendSensorStatus(Serial2); // gửi cho thiết bị đối tác
    
    // Gửi dữ liệu camera line cho Raspberry Pi
    sendLineReading(Serial);
    sendLineReading(Serial2);
  }

  // Các xử lý điều khiển động cơ khác có thể bổ sung sau
}
