import unittest

from loom_companion.sources.cpu.libre_hardware_monitor import LibreHardwareMonitorCpuTemperature
from loom_companion.sources.gpu.libre_hardware_monitor import LibreHardwareMonitorGpu
from loom_companion.sources.system.libre_hardware_monitor import (
    LibreHardwareMonitor,
    flatten_sensor_tree,
    parse_sensor_value,
)
from tests.fixtures import LHM_DESKTOP_WITH_AMD_GPU


class FakeClock:
    def __init__(self):
        self.now = 0.0

    def __call__(self):
        return self.now


def monitor_returning(tree, clock=None):
    calls = []

    def fetch(url, timeout):
        calls.append(url)
        return tree

    return LibreHardwareMonitor(fetch=fetch, clock=clock or FakeClock()), calls


class ParseSensorValueTest(unittest.TestCase):
    def test_reads_number_and_unit(self):
        self.assertEqual(parse_sensor_value("61.5 °C"), (61.5, "°C"))

    def test_accepts_comma_as_decimal_separator(self):
        self.assertEqual(parse_sensor_value("55,0 °C"), (55.0, "°C"))

    def test_accepts_thousands_separator(self):
        self.assertEqual(parse_sensor_value("12,288.0 MB"), (12288.0, "MB"))

    def test_rejects_text(self):
        self.assertIsNone(parse_sensor_value("Value"))
        self.assertIsNone(parse_sensor_value(""))


class FlattenSensorTreeTest(unittest.TestCase):
    def test_attaches_hardware_and_group_to_each_sensor(self):
        sensors = list(flatten_sensor_tree(LHM_DESKTOP_WITH_AMD_GPU))
        cpu_temperature = next(s for s in sensors if s.name == "Core (Tctl/Tdie)")
        self.assertEqual(cpu_temperature.hardware, "AMD Ryzen 7 5800X")
        self.assertEqual(cpu_temperature.group, "Temperatures")
        self.assertEqual(len(sensors), 11)


class LibreHardwareMonitorCachingTest(unittest.TestCase):
    def test_reuses_reading_within_cache_window(self):
        clock = FakeClock()
        monitor, calls = monitor_returning(LHM_DESKTOP_WITH_AMD_GPU, clock)
        monitor.sensors()
        clock.now = 0.5
        monitor.sensors()
        self.assertEqual(len(calls), 1)
        clock.now = 1.5
        monitor.sensors()
        self.assertEqual(len(calls), 2)

    def test_backs_off_when_not_running(self):
        clock = FakeClock()
        monitor, calls = monitor_returning(None, clock)
        self.assertFalse(monitor.is_running())
        clock.now = 5
        monitor.sensors()
        self.assertEqual(len(calls), 1)


class LibreHardwareMonitorSourcesTest(unittest.TestCase):
    def setUp(self):
        self.monitor, _ = monitor_returning(LHM_DESKTOP_WITH_AMD_GPU)

    def test_cpu_temperature(self):
        self.assertEqual(LibreHardwareMonitorCpuTemperature(self.monitor).read(), 61.5)

    def test_prefers_discrete_gpu_over_integrated(self):
        reading = LibreHardwareMonitorGpu(self.monitor).read()
        self.assertEqual(reading.load_percent, 87.0)
        self.assertEqual(reading.temperature_c, 58.0)
        self.assertAlmostEqual(reading.vram_used_gb, 6.0)
        self.assertAlmostEqual(reading.vram_total_gb, 12.0)
        self.assertAlmostEqual(reading.vram_percent, 50.0)

    def test_nothing_when_monitor_not_running(self):
        monitor, _ = monitor_returning(None)
        self.assertIsNone(LibreHardwareMonitorGpu(monitor).read())
        self.assertIsNone(LibreHardwareMonitorCpuTemperature(monitor).read())


if __name__ == "__main__":
    unittest.main()
