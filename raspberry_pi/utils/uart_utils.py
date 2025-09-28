"""Utility helpers shared by the Raspberry Pi UART/USB drivers.

The Raspberry Pi talks to multiple ESP32 boards.  Both boards expose a
serial interface – one over USB and one directly over the Pi's UART pins.
This module centralises the boilerplate required to open the serial port,
read and decode lines and send commands.  The helpers are intentionally
light‑weight so that they can be re-used both by the USB and the UART
drivers without duplicating code.
"""

from __future__ import annotations

import logging
from dataclasses import dataclass
from typing import Iterable, Optional

try:  # pragma: no cover - the dependency is not available in tests.
    import serial  # type: ignore
except ImportError:  # pragma: no cover - handled at runtime on the Pi.
    serial = None  # type: ignore


LOGGER = logging.getLogger("raspberry_pi.uart")


class SerialUnavailableError(RuntimeError):
    """Raised when pyserial is not installed but a serial port is requested."""


@dataclass
class SerialConfig:
    """Configuration for opening a serial port."""

    port: str
    baudrate: int = 115_200
    timeout: float = 0.1
    write_timeout: float = 0.1


def ensure_pyserial() -> None:
    """Ensure that the ``serial`` module is available."""

    if serial is None:  # pragma: no cover - only triggered on CI.
        raise SerialUnavailableError(
            "pyserial is required but could not be imported. Install the"
            " 'pyserial' package on the Raspberry Pi."
        )


def open_serial_port(config: SerialConfig):
    """Open and return a ``serial.Serial`` instance.

    Parameters
    ----------
    config:
        Serial configuration describing the port, baud rate and timeouts.
    """

    ensure_pyserial()
    LOGGER.debug(
        "Opening serial port %s at %d baud (timeout %.2fs)",
        config.port,
        config.baudrate,
        config.timeout,
    )
    ser = serial.Serial(  # type: ignore[call-arg]
        port=config.port,
        baudrate=config.baudrate,
        timeout=config.timeout,
        write_timeout=config.write_timeout,
    )
    try:
        ser.reset_input_buffer()
    except (serial.SerialException, AttributeError):  # pragma: no cover
        LOGGER.debug("Could not reset input buffer for port %s", config.port)
    return ser


def read_serial_line(ser) -> Optional[str]:
    """Read a single line from ``ser``.

    The function returns ``None`` if the read timed out or no serial object
    was supplied.  Returned strings are decoded using UTF-8 and trimmed.
    """

    if ser is None:
        return None
    try:
        raw = ser.readline()
    except (serial.SerialException, OSError) as exc:  # pragma: no cover
        LOGGER.error("Failed to read from serial port: %s", exc)
        return None
    if not raw:
        return None
    try:
        decoded = raw.decode("utf-8", errors="ignore").strip()
    except AttributeError:  # pragma: no cover - ser.readline may return str.
        decoded = raw.strip()
    if decoded:
        LOGGER.debug("RX <- %s", decoded)
    return decoded or None


def send_serial_command(ser, command: str, *, append_newline: bool = True) -> None:
    """Send ``command`` over ``ser``.

    The helper adds a newline by default to match the firmware protocol used
    by the ESP32 boards.
    """

    if ser is None:
        return
    payload = command if not append_newline else f"{command}\n"
    LOGGER.debug("TX -> %s", command)
    try:
        ser.write(payload.encode("utf-8"))
    except (serial.SerialException, AttributeError, OSError) as exc:
        LOGGER.error("Failed to send '%s': %s", command, exc)


def iter_available_lines(ser, limit: int = 25) -> Iterable[str]:
    """Yield up to ``limit`` lines currently buffered on ``ser``."""

    for _ in range(limit):
        if ser is None:
            break
        try:
            if hasattr(ser, "in_waiting") and ser.in_waiting == 0:  # type: ignore[attr-defined]
                break
        except (serial.SerialException, AttributeError):  # pragma: no cover
            break
        line = read_serial_line(ser)
        if line is None:
            break
        yield line


def close_serial_port(ser) -> None:
    """Close the provided serial object if possible."""

    if ser is None:
        return
    try:
        ser.close()
    except (serial.SerialException, AttributeError, OSError):  # pragma: no cover
        LOGGER.debug("Ignoring serial close failure")