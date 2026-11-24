"""CPU temperature from LibreHardwareMonitor (Windows)."""

from __future__ import annotations

from typing import Optional

from ..readings import CpuTemperatureSource
from ..system.libre_hardware_monitor import LibreHardwareMonitor

CPU_TEMPERATURE_NAMES = (
    "CPU Package",
    "Core (Tctl/Tdie)",
    "Tctl/Tdie",
    "Tdie",
    "Core Average",
    "CPU Die (average)",
)


class LibreHardwareMonitorCpuTemperature(CpuTemperatureSource):
    """Reads CPU temperature from LibreHardwareMonitor's sensor list."""
    description = "LibreHardwareMonitor"

    def __init__(self, monitor: LibreHardwareMonitor):
        self._monitor = monitor

    def read(self) -> Optional[float]:
        """Return the first °C temperature matching ``CPU_TEMPERATURE_NAMES``, or ``None``.

        Names are checked in priority order so a package or die temperature wins over
        per-core values. Returns ``None`` when LibreHardwareMonitor is not running.
        """
        temperatures = {}
        for sensor in self._monitor.sensors():
            if sensor.group == "Temperatures" and sensor.unit == "°C":
                temperatures.setdefault(sensor.name, sensor.value)
        for name in CPU_TEMPERATURE_NAMES:
            if name in temperatures:
                return temperatures[name]
        return None
