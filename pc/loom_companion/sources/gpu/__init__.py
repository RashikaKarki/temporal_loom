"""GPU sources, with selection of the best one for this machine."""

from ..readings import GpuSource
from ..system.libre_hardware_monitor import LibreHardwareMonitor
from ..system.platform_info import is_linux, is_macos, is_windows
from .amd_sysfs import AmdSysfsGpu
from .apple_ioreg import AppleIoregGpu
from .libre_hardware_monitor import LibreHardwareMonitorGpu
from .nvidia import NvidiaGpu


def detect_gpu_source(monitor: LibreHardwareMonitor, nvidia_device_index: int = 0) -> GpuSource:
    """Pick the GPU source for this machine.

    NVIDIA GPUs (via NVML) are preferred on any platform. Otherwise Windows uses
    LibreHardwareMonitor, macOS uses ioreg and Linux uses AMD sysfs. Falls back to
    a source that always reports ``None``.

    Args:
        monitor: Shared LibreHardwareMonitor client.
        nvidia_device_index: Which NVIDIA GPU to watch when several are present.
    """
    nvidia = NvidiaGpu.detect(nvidia_device_index)
    if nvidia:
        return nvidia
    if is_windows():
        return LibreHardwareMonitorGpu(monitor)
    if is_macos():
        return AppleIoregGpu.detect() or GpuSource()
    if is_linux():
        return AmdSysfsGpu.detect() or GpuSource()
    return GpuSource()


__all__ = ["detect_gpu_source"]
