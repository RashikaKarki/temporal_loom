"""AMD GPU stats from the Linux amdgpu sysfs interface under ``/sys/class/drm``."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Optional

from ..readings import BYTES_PER_GIB, GpuReading, GpuSource

DRM_ROOT = Path("/sys/class/drm")
CARD_NAME_PATTERN = re.compile(r"card\d+")
MILLIDEGREES_PER_DEGREE = 1000


def read_integer(path: Optional[Path]) -> Optional[int]:
    """Read an integer from a sysfs file, or return ``None`` if missing or unreadable."""
    if path is None:
        return None
    try:
        return int(path.read_text().strip())
    except (OSError, ValueError):
        return None


def find_temperature_file(device: Path) -> Optional[Path]:
    """Return the device's first hwmon ``temp1_input`` file, or ``None`` if there is none."""
    for hwmon in sorted((device / "hwmon").glob("hwmon*")):
        candidate = hwmon / "temp1_input"
        if candidate.exists():
            return candidate
    return None


class AmdSysfsGpu(GpuSource):
    """Reads load, temperature and VRAM for one amdgpu device from sysfs.

    Args:
        device: The card's ``device`` directory, e.g. ``/sys/class/drm/card0/device``.
    """
    def __init__(self, device: Path):
        self._device = device
        self._temperature_file = find_temperature_file(device)
        self.description = f"AMD GPU {device.parent.name} (sysfs)"

    @classmethod
    def detect(cls, drm_root: Path = DRM_ROOT) -> Optional["AmdSysfsGpu"]:
        """Return a source for the AMD GPU with the most VRAM, or ``None`` if none is found.

        Only cards exposing ``gpu_busy_percent`` are considered; choosing the largest VRAM
        favours a discrete GPU over an integrated one.
        """
        devices = [
            card / "device"
            for card in sorted(drm_root.glob("card*"))
            if CARD_NAME_PATTERN.fullmatch(card.name) and (card / "device" / "gpu_busy_percent").exists()
        ]
        if not devices:
            return None
        largest_vram = max(devices, key=lambda device: read_integer(device / "mem_info_vram_total") or 0)
        return cls(largest_vram)

    def read(self) -> Optional[GpuReading]:
        """Return the current reading, or ``None`` if the load cannot be read.

        Temperature is ``None`` when no hwmon sensor exists; missing VRAM values read as 0.
        """
        load = read_integer(self._device / "gpu_busy_percent")
        if load is None:
            return None
        millidegrees = read_integer(self._temperature_file)
        return GpuReading(
            load_percent=float(load),
            temperature_c=millidegrees / MILLIDEGREES_PER_DEGREE if millidegrees is not None else None,
            vram_used_gb=(read_integer(self._device / "mem_info_vram_used") or 0) / BYTES_PER_GIB,
            vram_total_gb=(read_integer(self._device / "mem_info_vram_total") or 0) / BYTES_PER_GIB,
        )
