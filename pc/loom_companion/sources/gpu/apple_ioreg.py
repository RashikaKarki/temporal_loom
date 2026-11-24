"""GPU stats on macOS from ``ioreg`` IOAccelerator performance statistics."""

from __future__ import annotations

import re
from typing import Callable, Optional, Sequence, Tuple

import psutil

from ..readings import BYTES_PER_GIB, GpuReading, GpuSource
from ..system.commands import run_command

IOREG_COMMAND = ("ioreg", "-r", "-d", "1", "-w", "0", "-c", "IOAccelerator")
UTILIZATION_PATTERN = re.compile(r'"Device Utilization %"\s*=\s*(\d+)')
DEDICATED_USED_PATTERN = re.compile(r'"vramUsedBytes"\s*=\s*(\d+)')
DEDICATED_FREE_PATTERN = re.compile(r'"vramFreeBytes"\s*=\s*(\d+)')
UNIFIED_IN_USE_PATTERN = re.compile(r'"In use system memory"\s*=\s*(\d+)')

CommandRunner = Callable[[Sequence[str]], Optional[str]]


def total_system_memory_bytes() -> int:
    """Return total system RAM in bytes, used as the GPU memory total on unified-memory Macs."""
    return psutil.virtual_memory().total


class AppleIoregGpu(GpuSource):
    """Reads GPU utilization and memory from ``ioreg``; temperature is not available.

    Args:
        run: Command runner returning stdout or ``None``, injectable for tests.
        system_memory: Function returning total RAM in bytes, injectable for tests.
    """
    description = "Apple GPU (ioreg)"

    def __init__(self, run: CommandRunner = run_command, system_memory: Callable[[], int] = total_system_memory_bytes):
        self._run = run
        self._system_memory = system_memory

    @classmethod
    def detect(cls, run: CommandRunner = run_command) -> Optional["AppleIoregGpu"]:
        """Return a source if ioreg currently reports GPU utilization, otherwise ``None``."""
        source = cls(run)
        return source if source.read() is not None else None

    def read(self) -> Optional[GpuReading]:
        """Return the current reading, or ``None`` if ioreg reports no utilization.

        With several accelerators, the highest utilization is reported.
        """
        report = self._run(IOREG_COMMAND)
        utilizations = [int(value) for value in UTILIZATION_PATTERN.findall(report or "")]
        if not utilizations:
            return None
        used_bytes, total_bytes = self._memory_bytes(report)
        return GpuReading(
            load_percent=float(max(utilizations)),
            temperature_c=None,
            vram_used_gb=used_bytes / BYTES_PER_GIB,
            vram_total_gb=total_bytes / BYTES_PER_GIB,
        )

    def _memory_bytes(self, report: str) -> Tuple[int, int]:
        """Return ``(used, total)`` GPU memory in bytes.

        Uses dedicated VRAM counters when present (Intel Macs with discrete GPUs), else
        unified memory in use against total system RAM (Apple silicon), else ``(0, 0)``.
        """
        dedicated_used = DEDICATED_USED_PATTERN.search(report)
        dedicated_free = DEDICATED_FREE_PATTERN.search(report)
        if dedicated_used and dedicated_free:
            used = int(dedicated_used.group(1))
            return used, used + int(dedicated_free.group(1))
        unified_in_use = UNIFIED_IN_USE_PATTERN.search(report)
        if unified_in_use:
            return int(unified_in_use.group(1)), self._system_memory()
        return 0, 0
