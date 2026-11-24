"""Terminal output helpers for status lines, source descriptions and serial port listings."""

from __future__ import annotations

import sys
from typing import Dict, Iterable, Optional

from ..core.snapshot import Snapshot
from ..transport.ports import SerialPortInfo


def format_temperature(celsius: Optional[float]) -> str:
    """Format a temperature in °C as a whole number like ``61C``, or ``--`` when unknown."""
    return f"{celsius:.0f}C" if celsius is not None else "--"


def format_gpu(snapshot: Snapshot) -> str:
    """Format GPU load, temperature and VRAM usage, or a N/A placeholder when no GPU was read."""
    gpu = snapshot.gpu
    if gpu is None:
        return "GPU   N/A      | VRAM  N/A"
    return f"GPU {gpu.load_percent:3.0f}% {format_temperature(gpu.temperature_c):>4} | VRAM {gpu.vram_percent:3.0f}%"


def format_status(snapshot: Snapshot) -> str:
    """Format a snapshot as a single fixed-width status line."""
    return (
        f"CPU {snapshot.cpu_load_percent:3.0f}% {format_temperature(snapshot.cpu_temperature_c):>4} | "
        f"{format_gpu(snapshot)} | RAM {snapshot.memory.percent:3.0f}% | "
        f"{snapshot.top_process.name:<20}"
    )


def rewrite_line(text: str) -> None:
    """Overwrite the current terminal line in place using a carriage return."""
    sys.stdout.write("\r" + text)
    sys.stdout.flush()


def announce(text: str) -> None:
    """Print a message on its own line so it is not overwritten by the status line."""
    sys.stdout.write("\n" + text + "\n")
    sys.stdout.flush()


def show_sources(sources: Dict[str, str]) -> None:
    """Print which source supplies each kind of reading."""
    for label, description in sources.items():
        print(f"{label:<16} {description}")


def show_ports(ports: Iterable[SerialPortInfo]) -> None:
    """Print available serial ports, marking those with a known USB serial chip as likely M5Stacks."""
    ports = list(ports)
    if not ports:
        print("No serial ports found.")
    for port in ports:
        vendor = f"0x{port.vendor_id:04X}" if port.vendor_id is not None else "----"
        marker = "  <- likely M5Stack" if port.is_known_usb_serial_chip else ""
        print(f"{port.device:<24} vendor {vendor}  {port.description}{marker}")
