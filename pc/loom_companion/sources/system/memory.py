"""System RAM usage via psutil."""

from __future__ import annotations

from dataclasses import dataclass

import psutil

from ..readings import BYTES_PER_GIB


@dataclass(frozen=True)
class MemoryUsage:
    """System RAM usage.

    Attributes:
        percent: Percentage of RAM in use, 0-100.
        used_gb: RAM in use (total minus available), in GiB.
        total_gb: Total RAM in GiB.
    """
    percent: float = 0.0
    used_gb: float = 0.0
    total_gb: float = 0.0


def read_memory() -> MemoryUsage:
    """Return current RAM usage."""
    memory = psutil.virtual_memory()
    used_bytes = memory.total - memory.available
    return MemoryUsage(
        percent=memory.percent,
        used_gb=used_bytes / BYTES_PER_GIB,
        total_gb=memory.total / BYTES_PER_GIB,
    )
