"""Serial connection to the M5Stack that reconnects on demand."""

from __future__ import annotations

from typing import Optional

import serial

from ..config import DEFAULT_BAUD_RATE
from .ports import find_device_port

WRITE_TIMEOUT_SECONDS = 1.0


def open_without_resetting(port: str, baud_rate: int) -> serial.Serial:
    """Open a serial port with DTR and RTS held low.

    On the M5Stack these lines drive the ESP32 reset and boot pins, so leaving them
    low keeps the display from rebooting when the port opens.
    """
    connection = serial.Serial()
    connection.port = port
    connection.baudrate = baud_rate
    connection.write_timeout = WRITE_TIMEOUT_SECONDS
    connection.dtr = False
    connection.rts = False
    connection.open()
    return connection


class DeviceLink:
    """Serial link to the M5Stack that connects lazily and drops the connection on errors.

    Can be used as a context manager to close the port on exit.

    Args:
        port: Serial port to use, or ``None`` to auto-detect on each connection attempt.
        baud_rate: Serial speed.
    """
    def __init__(self, port: Optional[str] = None, baud_rate: int = DEFAULT_BAUD_RATE):
        self._requested_port = port
        self._baud_rate = baud_rate
        self._connection: Optional[serial.Serial] = None
        self._connected_port: Optional[str] = None

    @property
    def is_connected(self) -> bool:
        """Whether a serial connection is currently open."""
        return self._connection is not None

    @property
    def port_name(self) -> Optional[str]:
        """The connected port, else the requested port, else ``None``."""
        return self._connected_port or self._requested_port

    def connect(self) -> bool:
        """Open the requested or auto-detected port.

        Returns:
            ``True`` on success, ``False`` if no port is found or it cannot be opened.
        """
        port = self._requested_port or find_device_port()
        if port is None:
            return False
        try:
            self._connection = open_without_resetting(port, self._baud_rate)
        except (serial.SerialException, OSError):
            return False
        self._connected_port = port
        return True

    def send(self, payload: bytes) -> bool:
        """Write a payload, connecting first if needed.

        Returns:
            ``True`` if the payload was written. On a write error the connection is
            closed so the next call reconnects.
        """
        if not self.is_connected and not self.connect():
            return False
        try:
            self._connection.write(payload)
            return True
        except (serial.SerialException, OSError):
            self.close()
            return False

    def close(self) -> None:
        """Close the connection if open, ignoring errors."""
        if self._connection is not None:
            try:
                self._connection.close()
            except (serial.SerialException, OSError):
                pass
        self._connection = None
        self._connected_port = None

    def __enter__(self) -> "DeviceLink":
        return self

    def __exit__(self, *exception_info) -> None:
        self.close()
