"""Wire format sent to the M5Stack: one compact JSON object per line, UTF-8 encoded.

Percentages and temperatures are whole numbers, with -1 meaning unavailable.
"""

from __future__ import annotations

import json
from typing import Optional

from ..core.snapshot import Snapshot
from ..sources.readings import GpuReading

UNAVAILABLE = -1
NO_GPU = GpuReading(load_percent=UNAVAILABLE)


def encode_optional(value: Optional[float]) -> int:
    """Round a value to an integer, or return -1 when it is ``None``."""
    return round(value) if value is not None else UNAVAILABLE


def to_message(snapshot: Snapshot) -> dict:
    """Convert a snapshot to the message dictionary.

    Keys: ``cpu``/``gpu``/``ram``/``vram`` load percentages, ``ctmp``/``gtmp``
    temperatures in °C, ``ramu``/``ramt`` and ``vramu``/``vramt`` used and total
    memory in GiB, ``proc``/``pp`` top process name and CPU percent, and ``clk``
    local time. Without a GPU, GPU load, temperature and VRAM percent are -1 and
    VRAM sizes are 0.
    """
    gpu = snapshot.gpu or NO_GPU
    has_gpu = snapshot.gpu is not None
    return {
        "cpu": round(snapshot.cpu_load_percent),
        "ctmp": encode_optional(snapshot.cpu_temperature_c),
        "gpu": round(gpu.load_percent),
        "gtmp": encode_optional(gpu.temperature_c),
        "ram": round(snapshot.memory.percent),
        "vram": round(gpu.vram_percent) if has_gpu else UNAVAILABLE,
        "ramu": round(snapshot.memory.used_gb, 1),
        "ramt": round(snapshot.memory.total_gb),
        "vramu": round(gpu.vram_used_gb, 1) if has_gpu else 0,
        "vramt": round(gpu.vram_total_gb) if has_gpu else 0,
        "proc": snapshot.top_process.name,
        "pp": round(snapshot.top_process.percent),
        "clk": snapshot.clock,
    }


def encode(snapshot: Snapshot) -> bytes:
    """Encode a snapshot as a newline-terminated compact JSON line."""
    return (json.dumps(to_message(snapshot), separators=(",", ":")) + "\n").encode("utf-8")
