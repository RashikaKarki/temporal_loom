"""CPU temperature from kernel hardware sensors via psutil (Linux and other Unix systems)."""

from __future__ import annotations

from typing import Callable, Dict, List, Optional

import psutil

from ..readings import CpuTemperatureSource

PREFERRED_CHIP_LABELS = (
    ("coretemp", "Package id"),
    ("k10temp", "Tctl"),
    ("k10temp", "Tdie"),
    ("zenpower", "Tdie"),
    ("cpu_thermal", ""),
    ("cpu-thermal", ""),
    ("soc_thermal", ""),
    ("acpitz", ""),
)


def read_psutil_sensors() -> Dict[str, List]:
    """Return psutil's temperature sensors by chip name, or an empty dict if unsupported or failing."""
    if not hasattr(psutil, "sensors_temperatures"):
        return {}
    try:
        return psutil.sensors_temperatures() or {}
    except (OSError, RuntimeError):
        return {}


class KernelSensorCpuTemperature(CpuTemperatureSource):
    """Reads CPU temperature from the first matching chip in ``PREFERRED_CHIP_LABELS``.

    Args:
        read_sensors: Function returning sensors by chip name, injectable for tests.
    """
    description = "kernel sensors"

    def __init__(self, read_sensors: Callable[[], Dict[str, List]] = read_psutil_sensors):
        self._read_sensors = read_sensors

    @classmethod
    def detect(cls, read_sensors: Callable[[], Dict[str, List]] = read_psutil_sensors) -> Optional["KernelSensorCpuTemperature"]:
        """Return a source if it currently yields a temperature, otherwise ``None``."""
        source = cls(read_sensors)
        return source if source.read() is not None else None

    def read(self) -> Optional[float]:
        """Return the CPU temperature in °C from the most preferred chip and label, or ``None``."""
        chips = self._read_sensors()
        for chip, label_prefix in PREFERRED_CHIP_LABELS:
            for entry in chips.get(chip, []):
                if entry.label.startswith(label_prefix) and entry.current:
                    return float(entry.current)
        return None
