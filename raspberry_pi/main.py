import serial, time

# ESP32-1 qua USB (bánh sau, IR trái)
esp1 = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
time.sleep(2)

# ESP32-2 qua UART GPIO14/15 (bánh trước, IR phải)
esp2 = serial.Serial('/dev/serial0', 115200, timeout=1)
time.sleep(2)

def send_cmd(ser, name, cmd):
    ser.write((cmd + "\n").encode())
    print(f"📤 Sent to {name}: {cmd}")

def read_ir(ser):
    line = ser.readline().decode().strip()
    if "IR_LEFT:" in line:
        return ("LEFT", int(line.split(":")[1]))
    elif "IR_RIGHT:" in line:
        return ("RIGHT", int(line.split(":")[1]))
    return (None, None)

try:
    print("🤖 Xe bắt đầu dò line...")

    while True:
        left_name, left_val = read_ir(esp1)
        right_name, right_val = read_ir(esp2)

        if left_name and right_name:
            print(f"IR_LEFT={left_val}, IR_RIGHT={right_val}")

            if left_val == 0 and right_val == 0:
                send_cmd(esp1, "ESP32-1", "FORWARD")
                send_cmd(esp2, "ESP32-2", "FORWARD")

            elif left_val == 0 and right_val == 1:
                send_cmd(esp1, "ESP32-1", "RIGHT")
                send_cmd(esp2, "ESP32-2", "RIGHT")

            elif left_val == 1 and right_val == 0:
                send_cmd(esp1, "ESP32-1", "LEFT")
                send_cmd(esp2, "ESP32-2", "LEFT")

            else:  # cả hai đều 1 (ra khỏi line)
                send_cmd(esp1, "ESP32-1", "STOP")
                send_cmd(esp2, "ESP32-2", "STOP")

        time.sleep(0.05)

except KeyboardInterrupt:
    send_cmd(esp1, "ESP32-1", "STOP")
    send_cmd(esp2, "ESP32-2", "STOP")
    esp1.close()
    esp2.close()
    print("\n⛔ Dừng xe.")
