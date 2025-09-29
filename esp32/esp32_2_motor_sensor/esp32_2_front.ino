#include "motor_control.h"
#include "ir_sensor.h"
#include "ultrasonic_sensor.h"
#include "servo_control.h"
#include "uart_comm.h"

#include <ctype.h>

// ---------------------------------------------------------------------------
//  Cấu hình và trạng thái né vật cản bằng servo trước
// ---------------------------------------------------------------------------

enum AvoidState {
  AVOID_IDLE = 0,
  AVOID_WAIT_RIGHT,
  AVOID_WAIT_LEFT,
  AVOID_DECIDE,
  AVOID_EXEC_TURN,
  AVOID_EXEC_FORWARD,
  AVOID_EXEC_RETURN,
  AVOID_RECOVER,
  AVOID_BLOCKED
};

enum EscapePlan {
  ESCAPE_NONE = 0,
  ESCAPE_LEFT,
  ESCAPE_RIGHT
};

static AvoidState avoidState = AVOID_IDLE;
static EscapePlan escapePlan = ESCAPE_NONE;
static unsigned long stateEnteredAt = 0;
static int scanRightDistance = -1;
static int scanLeftDistance = -1;
static int lastFrontDistance = -1;

constexpr int DEFAULT_SPEED = 170;
constexpr int OBSTACLE_THRESHOLD_CM = 25;
constexpr unsigned long SERVO_SETTLE_MS = 400;
constexpr unsigned long TURN_DURATION_MS = 550;
constexpr unsigned long FORWARD_DURATION_MS = 750;
constexpr unsigned long RETURN_DURATION_MS = 550;
constexpr unsigned long RESCAN_INTERVAL_MS = 1200;

static void enterAvoidState(AvoidState newState) {
  avoidState = newState;
  stateEnteredAt = millis();
}

static bool avoidanceActive() {
  return avoidState != AVOID_IDLE;
}

static void handleManualCommand(char rawCmd) {
  char cmd = toupper(rawCmd);
  switch (cmd) {
    case 'F': moveForward(DEFAULT_SPEED);  break;
    case 'B': moveBackward(DEFAULT_SPEED); break;
    case 'L': turnLeft(DEFAULT_SPEED);     break;
    case 'R': turnRight(DEFAULT_SPEED);    break;
    case 'S': stopMotors();                break;
    default:  break;
  }
}

static void updateAvoidanceLogic() {
  unsigned long now = millis();

  switch (avoidState) {
    case AVOID_IDLE:
      if (getServoAngle() != 90) {
        setServoAngle(90);
      }
      lastFrontDistance = readUltrasonic(SENSOR_FRONT);
      if (lastFrontDistance != -1 && lastFrontDistance <= OBSTACLE_THRESHOLD_CM) {
        stopMotors();
        scanRightDistance = -1;
        scanLeftDistance = -1;
        escapePlan = ESCAPE_NONE;
        setServoAngle(0);  // quay sang phải trước
        enterAvoidState(AVOID_WAIT_RIGHT);
      }
      break;

    case AVOID_WAIT_RIGHT:
      if (now - stateEnteredAt >= SERVO_SETTLE_MS) {
        scanRightDistance = readUltrasonic(SENSOR_FRONT);
        lastFrontDistance = scanRightDistance;
        setServoAngle(180);  // quét sang trái
        enterAvoidState(AVOID_WAIT_LEFT);
      }
      break;

    case AVOID_WAIT_LEFT:
      if (now - stateEnteredAt >= SERVO_SETTLE_MS) {
        scanLeftDistance = readUltrasonic(SENSOR_FRONT);
        lastFrontDistance = scanLeftDistance;
        setServoAngle(90);  // đưa servo về trước
        enterAvoidState(AVOID_DECIDE);
      }
      break;

    case AVOID_DECIDE: {
        bool rightClear = (scanRightDistance == -1) || (scanRightDistance > OBSTACLE_THRESHOLD_CM);
        bool leftClear = (scanLeftDistance == -1) || (scanLeftDistance > OBSTACLE_THRESHOLD_CM);

        if (leftClear) {
          escapePlan = ESCAPE_LEFT;  // ưu tiên né về bên trái
          turnLeft(DEFAULT_SPEED);
          enterAvoidState(AVOID_EXEC_TURN);
        } else if (rightClear) {
          escapePlan = ESCAPE_RIGHT;
          turnRight(DEFAULT_SPEED);
          enterAvoidState(AVOID_EXEC_TURN);
        } else {
          escapePlan = ESCAPE_NONE;
          stopMotors();
          enterAvoidState(AVOID_BLOCKED);
        }
      }
      break;

    case AVOID_EXEC_TURN:
      if (now - stateEnteredAt >= TURN_DURATION_MS) {
        moveForward(DEFAULT_SPEED);
        enterAvoidState(AVOID_EXEC_FORWARD);
      }
      break;

    case AVOID_EXEC_FORWARD:
      if (now - stateEnteredAt >= FORWARD_DURATION_MS) {
        if (escapePlan == ESCAPE_LEFT) {
          turnRight(DEFAULT_SPEED);  // quay lại về line
        } else if (escapePlan == ESCAPE_RIGHT) {
          turnLeft(DEFAULT_SPEED);
        } else {
          stopMotors();
        }
        enterAvoidState(AVOID_EXEC_RETURN);
      }
      break;

    case AVOID_EXEC_RETURN:
      if (now - stateEnteredAt >= RETURN_DURATION_MS) {
        stopMotors();
        enterAvoidState(AVOID_RECOVER);
      }
      break;

    case AVOID_RECOVER:
      if (now - stateEnteredAt >= SERVO_SETTLE_MS) {
        setServoAngle(90);
        lastFrontDistance = readUltrasonic(SENSOR_FRONT);
        escapePlan = ESCAPE_NONE;
        enterAvoidState(AVOID_IDLE);
      }
      break;

    case AVOID_BLOCKED:
      if (getServoAngle() != 90) {
        setServoAngle(90);
      }
      lastFrontDistance = readUltrasonic(SENSOR_FRONT);
      if (lastFrontDistance == -1 || lastFrontDistance > OBSTACLE_THRESHOLD_CM) {
        enterAvoidState(AVOID_IDLE);
      } else if (now - stateEnteredAt >= RESCAN_INTERVAL_MS) {
        setServoAngle(0);
        enterAvoidState(AVOID_WAIT_RIGHT);
      }
      break;
  }
}

// ---------------------------------------------------------------------------
//  Thiết lập & vòng lặp chính
// ---------------------------------------------------------------------------  
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
    while (Serial2.available()) {
    char incoming = (char)Serial2.read();
    if (incoming == '\r' || incoming == '\n') {
      continue;
    }

    if (avoidanceActive()) {
      if (toupper(incoming) == 'S') {
        stopMotors();
        escapePlan = ESCAPE_NONE;
        setServoAngle(90);
        enterAvoidState(AVOID_IDLE);
      }
      continue;  // Đang né vật cản, bỏ qua lệnh khác từ Pi
    }

    handleManualCommand(incoming);
  }

  // Có thể gửi dữ liệu về Pi nếu muốn
  
    updateAvoidanceLogic();

  int distRight = readUltrasonic(SENSOR_RIGHT);
  bool irRight = readIRRight();
   uartSendStatus(lastFrontDistance, distRight, irRight);

  delay(50);
}
