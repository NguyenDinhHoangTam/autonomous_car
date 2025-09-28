#include "uart_comm.h"
#include "motor_control.h"

static String rxBuf;

void uartInit(unsigned long baud) {
  // UART2: RX=16, TX=17
  Serial2.begin(baud, SERIAL_8N1, 16, 17);
}

static void processLine(const String& line) {
  if (line.length() == 0) return;

  char cmd = toupper(line.charAt(0));
  int spd = 150; // mặc định
  if (line.length() > 1) {
    // lấy phần số phía sau
    spd = line.substring(1).toInt();
    if (spd <= 0) spd = 150;
    if (spd > 255) spd = 255;
  }

  switch (cmd) {
    case 'F': moveForward(spd);  break;
    case 'B': moveBackward(spd); break;
    case 'L': turnLeft(spd);     break;
    case 'R': turnRight(spd);    break;
    case 'S': stopMotors();      break;
    default:
      // không hợp lệ, bỏ qua
      break;
  }
}

void uartHandle() {
  while (Serial2.available()) {
    char c = (char)Serial2.read();
    if (c == '\r') continue;
    if (c == '\n') {
      processLine(rxBuf);
      rxBuf = "";
    } else {
      if (rxBuf.length() < 64) rxBuf += c; // tránh quá dài
    }
  }
}

void uartSendStatus(int distFront, int distRight, bool irRight) {
  Serial2.print("STAT,");
  Serial2.print(distFront);
  Serial2.print(',');
  Serial2.print(distRight);
  Serial2.print(',');
  Serial2.print(irRight ? 1 : 0);
  Serial2.print('\n');
}
