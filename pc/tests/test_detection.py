import unittest
from unittest import mock

from loom_companion.sources import cpu, gpu
from loom_companion.sources.cpu.libre_hardware_monitor import LibreHardwareMonitorCpuTemperature
from loom_companion.sources.gpu.libre_hardware_monitor import LibreHardwareMonitorGpu
from loom_companion.sources.readings import CpuTemperatureSource, GpuSource
from loom_companion.sources.system.libre_hardware_monitor import LibreHardwareMonitor


def pretend_platform(module, name):
    return mock.patch.multiple(
        module, is_windows=lambda: name == "windows", is_macos=lambda: name == "macos",
        **({"is_linux": lambda: name == "linux"} if hasattr(module, "is_linux") else {}),
    )


class GpuDetectionTest(unittest.TestCase):
    def setUp(self):
        self.monitor = LibreHardwareMonitor(fetch=lambda url, timeout: None)

    def test_nvidia_wins_on_every_platform(self):
        nvidia = object()
        for platform in ("windows", "macos", "linux"):
            with pretend_platform(gpu, platform), mock.patch.object(gpu.NvidiaGpu, "detect", return_value=nvidia):
                self.assertIs(gpu.detect_gpu_source(self.monitor), nvidia)

    def test_windows_without_nvidia_uses_libre_hardware_monitor(self):
        with pretend_platform(gpu, "windows"), mock.patch.object(gpu.NvidiaGpu, "detect", return_value=None):
            self.assertIsInstance(gpu.detect_gpu_source(self.monitor), LibreHardwareMonitorGpu)

    def test_macos_without_nvidia_uses_ioreg(self):
        apple = object()
        with pretend_platform(gpu, "macos"), mock.patch.object(gpu.NvidiaGpu, "detect", return_value=None), \
                mock.patch.object(gpu.AppleIoregGpu, "detect", return_value=apple):
            self.assertIs(gpu.detect_gpu_source(self.monitor), apple)

    def test_linux_without_supported_gpu_falls_back_gracefully(self):
        with pretend_platform(gpu, "linux"), mock.patch.object(gpu.NvidiaGpu, "detect", return_value=None), \
                mock.patch.object(gpu.AmdSysfsGpu, "detect", return_value=None):
            source = gpu.detect_gpu_source(self.monitor)
        self.assertIs(type(source), GpuSource)
        self.assertIsNone(source.read())


class CpuTemperatureDetectionTest(unittest.TestCase):
    def setUp(self):
        self.monitor = LibreHardwareMonitor(fetch=lambda url, timeout: None)

    def test_windows_uses_libre_hardware_monitor(self):
        with pretend_platform(cpu, "windows"):
            self.assertIsInstance(cpu.detect_cpu_temperature_source(self.monitor), LibreHardwareMonitorCpuTemperature)

    def test_macos_without_tools_falls_back_gracefully(self):
        with pretend_platform(cpu, "macos"), mock.patch.object(cpu.SmcCommandCpuTemperature, "detect", return_value=None):
            self.assertIs(type(cpu.detect_cpu_temperature_source(self.monitor)), CpuTemperatureSource)

    def test_linux_uses_kernel_sensors(self):
        kernel = object()
        with pretend_platform(cpu, "linux"), mock.patch.object(cpu.KernelSensorCpuTemperature, "detect", return_value=kernel):
            self.assertIs(cpu.detect_cpu_temperature_source(self.monitor), kernel)


if __name__ == "__main__":
    unittest.main()
