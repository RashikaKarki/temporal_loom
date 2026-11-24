"""Serial port listing and M5Stack port auto-detection."""

from __future__ import annotations

from dataclasses import dataclass
from typing import List, Optional

from serial.tools import list_ports

from ..config import KNOWN_USB_SERIAL_VENDOR_IDS, USB_SERIAL_DESCRIPTION_HINTS

MACOS_CALLOUT_PREFIX = "/dev/cu."


@dataclass(frozen=True)
class SerialPortInfo:
    """Summary of one serial port.

    Attributes:
        device: Port path or name, e.g. ``COM5`` or ``/dev/cu.usbserial-0001``.
        description: Description reported by the OS.
        vendor_id: USB vendor ID, or ``None`` for non-USB ports.
    """
    device: str
    description: str
    vendor_id: Optional[int]

    @property
    def is_known_usb_serial_chip(self) -> bool:
        """Whether the vendor ID matches a USB serial chip used on M5Stack boards."""
        return self.vendor_id in KNOWN_USB_SERIAL_VENDOR_IDS

    @property
    def looks_like_usb_serial(self) -> bool:
        """Whether the description suggests a USB serial adapter."""
        description = self.description.lower()
        return any(hint in description for hint in USB_SERIAL_DESCRIPTION_HINTS)


def list_serial_ports() -> List[SerialPortInfo]:
    """Return all serial ports on this machine."""
    return [
        SerialPortInfo(device=port.device, description=port.description or "", vendor_id=port.vid)
        for port in list_ports.comports()
    ]


def choose_device_port(ports: List[SerialPortInfo]) -> Optional[str]:
    """Pick the most likely M5Stack port, or ``None``.

    Ports with a known vendor ID are preferred over ones matched by description.
    On macOS, ``/dev/cu.*`` callout devices are preferred over ``/dev/tty.*``.
    """
    candidates = [port for port in ports if port.is_known_usb_serial_chip] or [
        port for port in ports if port.looks_like_usb_serial
    ]
    if not candidates:
        return None
    callout_first = sorted(candidates, key=lambda port: not port.device.startswith(MACOS_CALLOUT_PREFIX))
    return callout_first[0].device


def find_device_port() -> Optional[str]:
    """Return the most likely M5Stack port on this machine, or ``None``."""
    return choose_device_port(list_serial_ports())
