"""Immutable snapshot of all stats at one point in time."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from ..sources.readings import GpuReading
from ..sources.system.memory import MemoryUsage
from ..sources.system.processes import TopProcess


@dataclass(frozen=True)
class Snapshot:
    """All stats for one update.

    Attributes:
        cpu_load_percent: Overall CPU load, 0-100.
        cpu_temperature_c: CPU temperature in °C, or ``None`` if unavailable.
        gpu: GPU reading, or ``None`` if no GPU source produced data.
        memory: System RAM usage.
        top_process: Process using the most CPU.
        clock: Local wall-clock time as ``HH:MM``.
    """
    cpu_load_percent: float
    cpu_temperature_c: Optional[float]
    gpu: Optional[GpuReading]
    memory: MemoryUsage
    top_process: TopProcess
    clock: str
