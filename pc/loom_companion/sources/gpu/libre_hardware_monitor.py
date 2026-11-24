"""GPU stats from LibreHardwareMonitor (Windows)."""

from __future__ import annotations

from collections import defaultdict
from typing import Dict, List, Optional

from ..readings import GpuReading, GpuSource
from ..system.libre_hardware_monitor import HardwareSensor, LibreHardwareMonitor

LOAD_NAMES = ("GPU Core", "D3D 3D")
TEMPERATURE_NAMES = ("GPU Core", "GPU Hot Spot")
VRAM_USED_NAMES = ("GPU Memory Used", "D3D Dedicated Memory Used")
VRAM_TOTAL_NAMES = ("GPU Memory Total",)
VRAM_FREE_NAMES = ("GPU Memory Free",)
MEMORY_GROUPS = ("SmallData", "Data")
MEGABYTES_PER_GIGABYTE = 1024


def to_gigabytes(sensor: HardwareSensor) -> float:
    """Convert a memory sensor value to GiB, treating values not in MB as already in GiB."""
    return sensor.value / MEGABYTES_PER_GIGABYTE if sensor.unit.upper() == "MB" else sensor.value


class GpuSensorSet:
    """The LibreHardwareMonitor sensors belonging to one GPU."""
    def __init__(self, sensors: List[HardwareSensor]):
        self._sensors = sensors

    def first(self, groups, names) -> Optional[HardwareSensor]:
        """Return the first sensor in ``groups`` matching ``names``, trying names in priority order."""
        for name in names:
            for sensor in self._sensors:
                if sensor.group in groups and sensor.name == name:
                    return sensor
        return None

    def vram_total_gb(self) -> float:
        """Return total VRAM in GiB, derived from used plus free if needed, or 0 when unknown."""
        total = self.first(MEMORY_GROUPS, VRAM_TOTAL_NAMES)
        if total:
            return to_gigabytes(total)
        used, free = self.first(MEMORY_GROUPS, VRAM_USED_NAMES), self.first(MEMORY_GROUPS, VRAM_FREE_NAMES)
        return to_gigabytes(used) + to_gigabytes(free) if used and free else 0.0

    def reading(self) -> GpuReading:
        """Build a reading; missing load and VRAM default to 0 and missing temperature to ``None``."""
        load = self.first(("Load",), LOAD_NAMES)
        temperature = self.first(("Temperatures",), TEMPERATURE_NAMES)
        used = self.first(MEMORY_GROUPS, VRAM_USED_NAMES)
        return GpuReading(
            load_percent=load.value if load else 0.0,
            temperature_c=temperature.value if temperature else None,
            vram_used_gb=to_gigabytes(used) if used else 0.0,
            vram_total_gb=self.vram_total_gb(),
        )


def group_gpu_sensors_by_hardware(sensors: List[HardwareSensor]) -> Dict[str, GpuSensorSet]:
    """Group sensors by hardware name, keeping only hardware with ``GPU`` or ``D3D`` sensors."""
    by_hardware: Dict[str, List[HardwareSensor]] = defaultdict(list)
    for sensor in sensors:
        by_hardware[sensor.hardware].append(sensor)
    return {
        hardware: GpuSensorSet(hardware_sensors)
        for hardware, hardware_sensors in by_hardware.items()
        if any(sensor.name.startswith(("GPU", "D3D")) for sensor in hardware_sensors)
    }


class LibreHardwareMonitorGpu(GpuSource):
    """Reads the GPU from LibreHardwareMonitor, preferring the one with the most VRAM."""
    description = "LibreHardwareMonitor"

    def __init__(self, monitor: LibreHardwareMonitor):
        self._monitor = monitor

    def read(self) -> Optional[GpuReading]:
        """Return the reading for the GPU with the most VRAM, or ``None`` if no GPU is reported.

        Picking the largest VRAM favours a discrete GPU over integrated graphics.
        """
        gpus = group_gpu_sensors_by_hardware(self._monitor.sensors())
        if not gpus:
            return None
        discrete_first = max(gpus.values(), key=lambda gpu: gpu.vram_total_gb())
        return discrete_first.reading()
