"""USB serial driver for ESP32-1.

The first ESP32 is connected to the Raspberry Pi over USB.  It is responsible
for streaming line camera readings and providing auxiliary sensor data.  This
module exposes a small wrapper around the serial connection that understands
the text-based protocol used by the firmware.
"""

from __future__ import annotations

import logging
import time
from typing import Dict, List, Optional, Tuple

from .utils.uart_utils import (
    SerialConfig,
    close_serial_port,
    iter_available_lines,
    open_serial_port,
    send_serial_command,
)


LOGGER = logging.getLogger("raspberry_pi.esp32_usb")

VALID_DRIVE_COMMANDS = {"F", "B", "L", "R", "S"}


class USBToESP32:
    """Manage communication with ESP32-1 over USB."""

    def __init__(
        self,
        port: str = "/dev/ttyUSB0",
        baudrate: int = 921_600,
        timeout: float = 0.02,
    ) -> None:
        self._config = SerialConfig(port=port, baudrate=baudrate, timeout=timeout)
        self._serial = open_serial_port(self._config)
        self._latest_line: Optional[Dict[str, float]] = None
        self._last_sensor_packet: Optional[Dict[str, float | str]] = None

    # ------------------------------------------------------------------
    # Receiving helpers
    # ------------------------------------------------------------------
    def poll(self) -> Tuple[Optional[Dict[str, float]], List[Dict[str, float | str]]]:
        """Poll the serial port for new packets.

        Returns a tuple ``(line_data, sensor_packets)`` where ``line_data`` is
        the most recent line camera reading (if any) and ``sensor_packets`` is
        a list of auxiliary sensor dictionaries received during the poll.
        """

        sensor_packets: List[Dict[str, float | str]] = []
        line_data: Optional[Dict[str, float]] = None
        for line in iter_available_lines(self._serial):
            if line.startswith("LINE"):
                line_data = self._parse_line_packet(line)
            elif line.startswith("STAT"):
                stat_line, packet = self._parse_stat_packet(line)
                if stat_line:
                    line_data = stat_line
                if packet:
                    sensor_packets.append(packet)
                    self._last_sensor_packet = packet
            elif line.startswith("SENSOR"):
                packet = self._parse_sensor_packet(line)
                if packet:
                    sensor_packets.append(packet)
                    self._last_sensor_packet = packet
            else:
                LOGGER.debug("Unhandled line from ESP32-1: %s", line)
        if line_data:
            self._latest_line = line_data
        return line_data, sensor_packets

    def _parse_line_packet(self, line: str) -> Optional[Dict[str, float]]:
        # Expected format: ``LINE,<position>[,<width>]`` where the values are
        # normalised between 0 and 1.
        try:
            _, *payload = line.split(",")
            position = float(payload[0])
            width = float(payload[1]) if len(payload) > 1 else None
        except (ValueError, IndexError) as exc:
            LOGGER.warning("Failed to parse line camera packet '%s': %s", line, exc)
            return None
        packet: Dict[str, float] = {"position": position}
        if width is not None:
            packet["width"] = width
        packet["timestamp"] = time.time()
        LOGGER.debug("Parsed line camera packet: %s", packet)
        return packet

    def _parse_sensor_packet(self, line: str) -> Optional[Dict[str, float | str]]:
        # Expected format: ``SENSOR,<key>=<value>,...`` similar to the status
        # packets from ESP32-2.
        _, *payload = line.split(",")
        if not payload:
            return None
        packet: Dict[str, float | str] = {}
        for token in payload:
            if "=" not in token:
                continue
            key, value = token.split("=", 1)
            packet[key.strip()] = _maybe_float(value)
        if packet:
            packet["timestamp"] = time.time()
            LOGGER.debug("Parsed sensor packet: %s", packet)
        return packet or None
    def _parse_stat_packet(
        self, line: str
    ) -> Tuple[Optional[Dict[str, float]], Optional[Dict[str, float | str]]]:
        _, *payload = line.split(",")
        timestamp = time.time()
        keys = ("ultrasonic_left_cm", "ultrasonic_rear_cm", "ir_left", "rear_servo_deg")
        sensor_packet: Dict[str, float | str] = {}
        for key, value in zip(keys, payload):
            sensor_packet[key] = _maybe_float(value)
        if sensor_packet:
            sensor_packet["timestamp"] = timestamp
            LOGGER.debug("Parsed STAT packet: %s", sensor_packet)

        line_data: Optional[Dict[str, float]] = None
        ir_value = sensor_packet.get("ir_left") if sensor_packet else None
        numeric_ir: Optional[float]
        if isinstance(ir_value, (int, float)):
            numeric_ir = float(ir_value)
        elif isinstance(ir_value, str):
            try:
                numeric_ir = float(ir_value)
            except ValueError:
                numeric_ir = None
        else:
            numeric_ir = None

        if numeric_ir is not None:
            # Map IR reading (1=line detected on the left) into the 0..1 range.
            position = 0.1 if numeric_ir >= 0.5 else 0.9
            line_data = {"position": position, "timestamp": timestamp}

        return line_data, sensor_packet or None

    # ------------------------------------------------------------------
    # Commands
    # ------------------------------------------------------------------
    def send_command(self, command: str) -> None:
        """Send an arbitrary command to the USB-connected ESP32."""

        send_serial_command(self._serial, command)

    def send_drive_command(self, command: str) -> None:
        """Mirror drive commands to the USB board if the firmware expects it."""

        normalized = command.upper().strip()[:1]
        if normalized not in VALID_DRIVE_COMMANDS:
            raise ValueError(
                f"Unsupported drive command '{command}'. Expected one of "
                f"{sorted(VALID_DRIVE_COMMANDS)}"
            )
        self.send_command(normalized)

    # ------------------------------------------------------------------
    # Utilities
    # ------------------------------------------------------------------
    @property
    def latest_line(self) -> Optional[Dict[str, float]]:
        return self._latest_line

    @property
    def last_sensor_packet(self) -> Optional[Dict[str, float | str]]:
        return self._last_sensor_packet

    def close(self) -> None:
        close_serial_port(self._serial)


def _maybe_float(value: str) -> float | str:
    try:
        return float(value)
    except ValueError:
        return value.strip()