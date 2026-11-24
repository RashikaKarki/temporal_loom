"""CPU load and temperature sources, with platform-based temperature source selection."""

from ..readings import CpuTemperatureSource
from ..system.libre_hardware_monitor import LibreHardwareMonitor
from ..system.platform_info import is_macos, is_windows
from .kernel_sensors import KernelSensorCpuTemperature
from .libre_hardware_monitor import LibreHardwareMonitorCpuTemperature
from .load import CpuLoadMeter
from .smc_command import SmcCommandCpuTemperature


def detect_cpu_temperature_source(monitor: LibreHardwareMonitor) -> CpuTemperatureSource:
    """Pick the CPU temperature source for this platform.

    Windows uses LibreHardwareMonitor, macOS uses an installed SMC command-line tool,
    and other systems use kernel sensors via psutil. Falls back to a source that always
    reports ``None`` when nothing usable is found.
    """
    if is_windows():
        return LibreHardwareMonitorCpuTemperature(monitor)
    if is_macos():
        return SmcCommandCpuTemperature.detect() or CpuTemperatureSource()
    return KernelSensorCpuTemperature.detect() or CpuTemperatureSource()


__all__ = ["CpuLoadMeter", "detect_cpu_temperature_source"]
