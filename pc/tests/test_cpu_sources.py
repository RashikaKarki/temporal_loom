import unittest
from collections import namedtuple

from loom_companion.sources.cpu.kernel_sensors import KernelSensorCpuTemperature
from loom_companion.sources.cpu.smc_command import SmcCommandCpuTemperature

Reading = namedtuple("Reading", "label current high critical")


class KernelSensorCpuTemperatureTest(unittest.TestCase):
    def test_prefers_intel_package_temperature(self):
        chips = {
            "acpitz": [Reading("", 30.0, None, None)],
            "coretemp": [Reading("Core 0", 50.0, 80, 100), Reading("Package id 0", 54.0, 80, 100)],
        }
        self.assertEqual(KernelSensorCpuTemperature(lambda: chips).read(), 54.0)

    def test_reads_amd_tctl(self):
        chips = {"k10temp": [Reading("Tctl", 62.25, None, None)]}
        self.assertEqual(KernelSensorCpuTemperature(lambda: chips).read(), 62.25)

    def test_reads_raspberry_pi(self):
        chips = {"cpu_thermal": [Reading("", 48.7, None, None)]}
        self.assertEqual(KernelSensorCpuTemperature(lambda: chips).read(), 48.7)

    def test_detect_returns_none_without_sensors(self):
        self.assertIsNone(KernelSensorCpuTemperature.detect(lambda: {}))


class SmcCommandCpuTemperatureTest(unittest.TestCase):
    def test_uses_first_installed_command_that_works(self):
        outputs = {"smctemp": None, "osx-cpu-temp": "52.3°C\n"}
        source = SmcCommandCpuTemperature.detect(
            run=lambda command: outputs[command[0]], installed=lambda program: True
        )
        self.assertEqual(source.description, "osx-cpu-temp command")
        self.assertEqual(source.read(), 52.3)

    def test_ignores_zero_reading(self):
        self.assertIsNone(SmcCommandCpuTemperature(("smctemp", "-c"), run=lambda command: "0.0").read())

    def test_none_when_nothing_installed(self):
        self.assertIsNone(SmcCommandCpuTemperature.detect(installed=lambda program: False))


if __name__ == "__main__":
    unittest.main()
