"""NVIDIA GPU stats via NVML (the optional ``pynvml`` package); works on any OS with the NVIDIA driver."""

from __future__ import annotations

from typing import Optional

from ..readings import BYTES_PER_GIB, GpuReading, GpuSource


def load_nvml():
    """Import and return the ``pynvml`` module, or ``None`` if it is not installed."""
    try:
        import pynvml
    except ImportError:
        return None
    return pynvml


class NvidiaGpu(GpuSource):
    """Reads load, temperature and memory for one NVIDIA GPU through NVML.

    Args:
        nvml: The ``pynvml`` module.
        handle: NVML device handle.
        name: Device name used in the description.
    """
    def __init__(self, nvml, handle, name: str):
        self._nvml = nvml
        self._handle = handle
        self.description = f"{name} (NVML)"

    @classmethod
    def detect(cls, device_index: int = 0, nvml=None) -> Optional["NvidiaGpu"]:
        """Initialise NVML and return a source for ``device_index``, or ``None`` if unavailable.

        Returns ``None`` when pynvml is missing, NVML fails to initialise, or the index
        is out of range.

        Args:
            device_index: Zero-based GPU index.
            nvml: NVML module to use instead of importing ``pynvml``, for tests.
        """
        nvml = nvml or load_nvml()
        if nvml is None:
            return None
        try:
            nvml.nvmlInit()
            if device_index >= nvml.nvmlDeviceGetCount():
                return None
            handle = nvml.nvmlDeviceGetHandleByIndex(device_index)
            name = nvml.nvmlDeviceGetName(handle)
        except (nvml.NVMLError, OSError):
            return None
        if isinstance(name, bytes):
            name = name.decode(errors="replace")
        return cls(nvml, handle, name)

    def read(self) -> Optional[GpuReading]:
        """Return the current reading, or ``None`` if any NVML query fails."""
        nvml = self._nvml
        try:
            utilization = nvml.nvmlDeviceGetUtilizationRates(self._handle)
            temperature = nvml.nvmlDeviceGetTemperature(self._handle, nvml.NVML_TEMPERATURE_GPU)
            memory = nvml.nvmlDeviceGetMemoryInfo(self._handle)
        except nvml.NVMLError:
            return None
        return GpuReading(
            load_percent=float(utilization.gpu),
            temperature_c=float(temperature),
            vram_used_gb=memory.used / BYTES_PER_GIB,
            vram_total_gb=memory.total / BYTES_PER_GIB,
        )
