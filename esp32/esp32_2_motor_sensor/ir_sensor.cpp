#include "ir_sensor.h"

void setupIRSensor() {
  pinMode(IR_RIGHT_PIN, INPUT);
}

bool readIRRight() {
  // Một số module IR line out LOW khi gặp vạch đen, một số thì HIGH.
  // Tạm thời trả về mức logic trực tiếp; sau khi test thực tế, bạn có thể đảo lại.
  return digitalRead(IR_RIGHT_PIN) == HIGH;
}
