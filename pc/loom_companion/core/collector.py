"""Gathers readings from all sources into snapshots."""

from __future__ import annotations

import time
from datetime import datetime
from typing import Callable, Dict, Optional

from ..config import SLOW_SENSOR_REFRESH_SECONDS
from ..sources.cpu import CpuLoadMeter, detect_cpu_temperature_source
from ..sources.gpu import detect_gpu_source
from ..sources.system.libre_hardware_monitor import LibreHardwareMonitor
from ..sources.system.memory import read_memory
from ..sources.system.processes import TopProcess, TopProcessTracker
from .snapshot import Snapshot


class StatsCollector:
    """Collects a full ``Snapshot`` from the sources detected for this machine.

    CPU load, GPU and memory are read on every call. CPU temperature and the top
    process are slower to read, so they are refreshed at most every
    ``SLOW_SENSOR_REFRESH_SECONDS`` and reused in between.

    Args:
        nvidia_device_index: Which NVIDIA GPU to watch when several are present.
        clock: Monotonic time function, injectable for tests.
    """
    def __init__(self, nvidia_device_index: int = 0, clock: Callable[[], float] = time.monotonic):
        monitor = LibreHardwareMonitor()
        self._clock = clock
        self._cpu_load = CpuLoadMeter()
        self._cpu_temperature = detect_cpu_temperature_source(monitor)
        self._gpu = detect_gpu_source(monitor, nvidia_device_index)
        self._processes = TopProcessTracker()
        self._slow_refresh_due_at = 0.0
        self._latest_cpu_temperature: Optional[float] = None
        self._latest_top_process = TopProcess()

    def describe_sources(self) -> Dict[str, str]:
        """Return a human-readable description of the CPU temperature and GPU sources in use."""
        return {
            "CPU temperature": self._cpu_temperature.description,
            "GPU": self._gpu.description,
        }

    def collect(self) -> Snapshot:
        """Read the current stats, refreshing the slow sensors if they are due."""
        self._refresh_slow_sensors_when_due()
        return Snapshot(
            cpu_load_percent=self._cpu_load.read(),
            cpu_temperature_c=self._latest_cpu_temperature,
            gpu=self._gpu.read(),
            memory=read_memory(),
            top_process=self._latest_top_process,
            clock=datetime.now().strftime("%H:%M"),
        )

    def _refresh_slow_sensors_when_due(self) -> None:
        """Re-read CPU temperature and the top process once the refresh interval has passed."""
        now = self._clock()
        if now < self._slow_refresh_due_at:
            return
        self._slow_refresh_due_at = now + SLOW_SENSOR_REFRESH_SECONDS
        self._latest_cpu_temperature = self._cpu_temperature.read()
        self._latest_top_process = self._processes.sample()
