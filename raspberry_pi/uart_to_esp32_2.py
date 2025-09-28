"""Driver for the ESP32 connected to the Raspberry Pi's UART pins.

The ESP32 on UART (referred to as *ESP32-2*) is responsible for the primary
drive motors.  The Raspberry Pi periodically polls the board for status
packets and issues simple direction commands (Forward/Left/Right/Stop).

The firmware reports status lines in the form ``STAT,<key>=<value>,...``.  The
``UARTToESP32`` class handles opening the serial port, parsing status packets
and pushing drive commands.
"""

from __future__ import annotations

import logging
from typing import Dict, Optional

from .utils.uart_utils import (
    SerialConfig,
    close_serial_port,
    iter_available_lines,
    open_serial_port,
    read_serial_line,
    send_serial_command,
)


LOGGER = logging.getLogger("raspberry_pi.esp32_uart")


class UARTToESP32:
    """High level interface to ESP32-2 via ``/dev/ttyS0``."""

    VALID_COMMANDS = {"F", "B", "L", "R", "S"}

    def __init__(
        self,
        port: str = "/dev/ttyS0",
        baudrate: int = 115_200,
        timeout: float = 0.05,
    ) -> None:
        self._config = SerialConfig(port=port, baudrate=baudrate, timeout=timeout)
        self._serial = open_serial_port(self._config)
        self._latest_status: Dict[str, float | str] = {}

    # ------------------------------------------------------------------
    # Reading helpers
    # ------------------------------------------------------------------
    def read_status_once(self) -> Optional[Dict[str, float | str]]:
        """Read and parse a single ``STAT`` packet if available."""

        line = read_serial_line(self._serial)
        if line is None:
            return None
        return self._parse_status_line(line)

    def poll_status(self) -> Optional[Dict[str, float | str]]:
        """Poll the serial port for the most recent ``STAT`` message."""

        status: Optional[Dict[str, float | str]] = None
        for line in iter_available_lines(self._serial):
            parsed = self._parse_status_line(line)
            if parsed is not None:
                status = parsed
        return status

    def _parse_status_line(self, line: str) -> Optional[Dict[str, float | str]]:
        if not line.startswith("STAT"):
            LOGGER.debug("Ignoring non status line from ESP32-2: %s", line)
            return None
        parts = line.strip().split(",")
        status: Dict[str, float | str] = {}
        positional_values: list[float] = []
        for token in parts[1:]:
            token = token.strip()
            if not token:
                continue
            if "=" not in token:
                maybe_value = _maybe_float(token)
                if isinstance(maybe_value, float):
                    positional_values.append(maybe_value)
                else:
                    status[token] = "1"
                continue
            key, value = token.split("=", 1)
            status[key.strip()] = _maybe_float(value)
            
        if positional_values:
            fallback_keys = ("front", "right", "ir_right")
            for key, value in zip(fallback_keys, positional_values):
                status.setdefault(key, value)
        if status:
            self._latest_status = status
            LOGGER.debug("Parsed ESP32-2 status: %s", status)
        return status or None

    # ------------------------------------------------------------------
    # Command helpers
    # ------------------------------------------------------------------
    def send_drive_command(self, command: str) -> None:
        """Send a direction command to the ESP32.

        Parameters
        ----------
        command:
            Single character command: ``F`` (forward), ``B`` (backward), ``L``
            (left), ``R`` (right) or ``S`` (stop).
        """

        normalized = command.upper().strip()[:1]
        if normalized not in self.VALID_COMMANDS:
            raise ValueError(
                f"Unsupported drive command '{command}'. Expected one of "
                f"{sorted(self.VALID_COMMANDS)}"
            )
        send_serial_command(self._serial, normalized)

    # ------------------------------------------------------------------
    # Utility helpers
    # ------------------------------------------------------------------
    @property
    def latest_status(self) -> Dict[str, float | str]:
        return self._latest_status

    def close(self) -> None:
        close_serial_port(self._serial)


def _maybe_float(value: str) -> float | str:
    """Try to convert ``value`` to ``float``."""

    try:
        return float(value)
    except ValueError:
        return value.strip()
        return value.strip()