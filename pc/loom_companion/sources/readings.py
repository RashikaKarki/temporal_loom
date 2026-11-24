"""Shared reading types and the null-object base classes for temperature and GPU sources."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

BYTES_PER_GIB = 1024**3


@dataclass(frozen=True)
class GpuReading:
    """One GPU sample.

    Attributes:
        load_percent: GPU utilization, 0-100.
        temperature_c: GPU temperature in °C, or ``None`` if the source cannot report it.
        vram_used_gb: Video memory in use, in GiB.
        vram_total_gb: Total video memory in GiB, or 0 when unknown.
    """
    load_percent: float = 0.0
    temperature_c: Optional[float] = None
    vram_used_gb: float = 0.0
    vram_total_gb: float = 0.0

    @property
    def vram_percent(self) -> float:
        """VRAM usage as a percentage capped at 100, or 0 when the total is unknown."""
        if self.vram_total_gb <= 0:
            return 0.0
        return min(100.0, self.vram_used_gb / self.vram_total_gb * 100)


class CpuTemperatureSource:
    """Base CPU temperature source; used as-is when no sensor is available."""
    description = "unavailable"

    def read(self) -> Optional[float]:
        """Return the CPU temperature in °C, or ``None`` if it cannot be read."""
        return None


class GpuSource:
    """Base GPU source; used as-is when no GPU can be monitored."""
    description = "unavailable"

    def read(self) -> Optional[GpuReading]:
        """Return the current GPU reading, or ``None`` if it cannot be read."""
        return None
