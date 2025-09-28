from pathlib import Path
import sys

import pytest

PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from raspberry_pi import uart_to_esp32_2
from raspberry_pi.uart_to_esp32_2 import UARTToESP32


def _make_uart_instance() -> UARTToESP32:
    instance = UARTToESP32.__new__(UARTToESP32)
    instance._serial = object()
    instance._config = None  # type: ignore[attr-defined]
    instance._latest_status = {}
    return instance


def test_parse_status_with_key_value_pairs():
    uart = _make_uart_instance()

    status = UARTToESP32._parse_status_line(
        uart, "STAT,front=123,right=45,ir_right=0"
    )

    assert status == {"front": 123.0, "right": 45.0, "ir_right": 0.0}
    assert uart.latest_status == status


def test_parse_status_with_positional_values():
    uart = _make_uart_instance()

    status = UARTToESP32._parse_status_line(uart, "STAT,123,45,0")

    assert status == {"front": 123.0, "right": 45.0, "ir_right": 0.0}
    assert uart.latest_status == status


def test_poll_status_returns_latest(monkeypatch: pytest.MonkeyPatch):
    uart = _make_uart_instance()

    def fake_iter_available_lines(_serial):
        yield "garbage"
        yield "STAT,front=99,right=88,ir_right=1"

    monkeypatch.setattr(uart_to_esp32_2, "iter_available_lines", fake_iter_available_lines)

    status = uart.poll_status()

    assert status == {"front": 99.0, "right": 88.0, "ir_right": 1.0}
    assert uart.latest_status == status
