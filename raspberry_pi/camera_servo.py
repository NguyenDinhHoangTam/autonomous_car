"""Camera servo control utilities for the Raspberry Pi."""

from __future__ import annotations

import logging
from dataclasses import dataclass


LOGGER = logging.getLogger("raspberry_pi.camera_servo")


@dataclass
class ServoConfig:
    pin: int = 18  # Default PWM pin (GPIO18 supports hardware PWM)
    frequency: int = 50
    min_pulse_width: int = 500  # microseconds
    max_pulse_width: int = 2500  # microseconds
    min_angle: float = 0.0
    max_angle: float = 180.0


class CameraServoController:
    """Control the line camera tilt servo."""

    def __init__(self, config: ServoConfig | None = None) -> None:
        self.config = config or ServoConfig()
        self._pigpio = self._init_pigpio()
        self._gpio_pwm = None
        if self._pigpio is None:
            self._gpio_pwm = self._init_rpi_gpio()
        LOGGER.info(
            "Initialised camera servo on GPIO %d using %s",
            self.config.pin,
            "pigpio" if self._pigpio else "RPi.GPIO" if self._gpio_pwm else "noop",
        )

    def _init_pigpio(self):
        try:  # pragma: no cover - dependency not available in CI.
            import pigpio

            pi = pigpio.pi()
            if not pi.connected:
                LOGGER.warning("pigpio daemon not running – falling back to RPi.GPIO")
                return None
            pi.set_mode(self.config.pin, pigpio.OUTPUT)
            return pi
        except (ImportError, OSError):  # pragma: no cover
            LOGGER.info("pigpio not available")
            return None

    def _init_rpi_gpio(self):
        try:  # pragma: no cover - dependency not available in CI.
            import RPi.GPIO as GPIO

            GPIO.setmode(GPIO.BCM)
            GPIO.setup(self.config.pin, GPIO.OUT)
            pwm = GPIO.PWM(self.config.pin, self.config.frequency)
            pwm.start(0)
            return pwm
        except (ImportError, RuntimeError):
            LOGGER.info("RPi.GPIO not available – servo control disabled")
            return None

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------
    def set_angle(self, angle: float) -> None:
        """Set the servo to ``angle`` degrees."""

        clamped = max(self.config.min_angle, min(self.config.max_angle, angle))
        pulse_width = self._angle_to_pulse_width(clamped)
        if self._pigpio:
            self._pigpio.set_servo_pulsewidth(self.config.pin, pulse_width)
        elif self._gpio_pwm:
            duty_cycle = self._pulse_width_to_duty_cycle(pulse_width)
            self._gpio_pwm.ChangeDutyCycle(duty_cycle)
        else:
            LOGGER.debug("Servo set_angle called but no PWM backend available")
        LOGGER.debug("Servo angle %.1f -> pulse %dus", clamped, pulse_width)

    def cleanup(self) -> None:
        if self._pigpio:
            self._pigpio.set_servo_pulsewidth(self.config.pin, 0)
            self._pigpio.stop()
        if self._gpio_pwm:
            try:  # pragma: no cover - depends on hardware libs.
                import RPi.GPIO as GPIO

                self._gpio_pwm.stop()
                GPIO.cleanup(self.config.pin)
            except ImportError:
                pass

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------
    def _angle_to_pulse_width(self, angle: float) -> int:
        span = self.config.max_angle - self.config.min_angle
        if span <= 0:
            raise ValueError("Invalid servo configuration: angle span must be positive")
        normalized = (angle - self.config.min_angle) / span
        pulse_span = self.config.max_pulse_width - self.config.min_pulse_width
        return int(self.config.min_pulse_width + normalized * pulse_span)

    def _pulse_width_to_duty_cycle(self, pulse_width: int) -> float:
        period_us = 1_000_000 / self.config.frequency
        duty_cycle = (pulse_width / period_us) * 100
        return duty_cycle


__all__ = ["CameraServoController", "ServoConfig"]