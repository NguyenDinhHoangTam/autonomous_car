"""Main control loop executed on the Raspberry Pi."""

from __future__ import annotations

import argparse
import logging
import signal
import sys
import time
from typing import Optional

from .camera_servo import CameraServoController
from .uart_to_esp32_2 import UARTToESP32
from .usb_to_esp32_1 import USBToESP32


LOGGER = logging.getLogger("raspberry_pi.main")
RUNNING = True


def signal_handler(signum, _frame):  # pragma: no cover - requires OS signals
    global RUNNING
    LOGGER.info("Received signal %s – shutting down", signum)
    RUNNING = False


def setup_logging(level: str = "INFO") -> None:
    numeric_level = getattr(logging, level.upper(), logging.INFO)
    logging.basicConfig(
        level=numeric_level,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    )


def parse_args(argv: Optional[list[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Autonomous car controller")
    parser.add_argument("--usb-port", default="/dev/ttyUSB0", help="ESP32-1 USB port")
    parser.add_argument("--uart-port", default="/dev/ttyS0", help="ESP32-2 UART port")
    parser.add_argument("--baud-usb", type=int, default=921_600, help="USB baudrate")
    parser.add_argument("--baud-uart", type=int, default=115_200, help="UART baudrate")
    parser.add_argument(
        "--loop-rate",
        type=float,
        default=30.0,
        help="Control loop rate in Hz",
    )
    parser.add_argument(
        "--log-level",
        default="INFO",
        help="Python logging level",
    )
    parser.add_argument(
        "--disable-servo",
        action="store_true",
        help="Disable camera servo control",
    )
    return parser.parse_args(argv)


def compute_drive_command(line_data: Optional[dict]) -> str:
    """Derive a motor command based on the line position."""

    if not line_data:
        return "S"
    position = line_data.get("position")
    if position is None:
        return "S"
    if position < 0.4:
        return "L"
    if position > 0.6:
        return "R"
    return "F"


def update_servo_target(servo: Optional[CameraServoController], line_data: Optional[dict]) -> None:
    if servo is None or not line_data:
        return
    position = line_data.get("position")
    if position is None:
        return
    # Map position 0..1 to +/-30 degrees around straight ahead.
    target_angle = 90.0 + (position - 0.5) * 60.0
    servo.set_angle(target_angle)


def main(argv: Optional[list[str]] = None) -> int:
    args = parse_args(argv)
    setup_logging(args.log_level)
    LOGGER.info("Starting Raspberry Pi control loop")

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    loop_interval = 1.0 / max(args.loop_rate, 1.0)
    servo = None if args.disable_servo else CameraServoController()
    usb = USBToESP32(port=args.usb_port, baudrate=args.baud_usb)
    uart = UARTToESP32(port=args.uart_port, baudrate=args.baud_uart)

    last_command = ""
    try:
        while RUNNING:
            loop_start = time.time()

            line_data, sensor_packets = usb.poll()
            if line_data is None:
                line_data = usb.latest_line
            if sensor_packets:
                for packet in sensor_packets:
                    LOGGER.debug("Sensor packet: %s", packet)

            status = uart.poll_status()
            if status:
                LOGGER.debug("ESP32-2 status: %s", status)

            command = compute_drive_command(line_data)
            if command != last_command:
                LOGGER.info(
                    "Drive command %s (line=%.3f)",
                    command,
                    line_data.get("position") if line_data else float("nan"),
                )
                uart.send_drive_command(command)
                usb.send_drive_command(command)
                last_command = command

            update_servo_target(servo, line_data)

            elapsed = time.time() - loop_start
            sleep_time = max(0.0, loop_interval - elapsed)
            time.sleep(sleep_time)
    finally:
        LOGGER.info("Stopping control loop")
        usb.close()
        uart.close()
        if servo:
            servo.cleanup()

    return 0


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    sys.exit(main())