#pragma once
#include <Arduino.h>

// Khởi tạo UART2 (GPIO16 RX, 17 TX)
void uartInit(unsigned long baud);

// Đọc lệnh từ Pi, parse và gọi hàm điều khiển động cơ
// Lệnh dạng:
//   F[spd]\n  (Forward)      ví dụ: F180
//   B[spd]\n  (Backward)
//   L[spd]\n  (Turn Left)
//   R[spd]\n  (Turn Right)
//   S\n       (Stop)
void uartHandle();

// Gửi trạng thái cảm biến về Pi dạng CSV 1 dòng:
//   STAT,<front_cm>,<right_cm>,<ir_right>\n
void uartSendStatus(int distFront, int distRight, bool irRight);
