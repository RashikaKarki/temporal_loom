import tempfile
import unittest
from pathlib import Path

from loom_companion.sources.gpu.amd_sysfs import AmdSysfsGpu
from loom_companion.sources.gpu.apple_ioreg import AppleIoregGpu
from loom_companion.sources.gpu.nvidia import NvidiaGpu
from loom_companion.sources.readings import BYTES_PER_GIB
from tests.fixtures import IOREG_APPLE_SILICON, IOREG_INTEL_MAC_WITH_RADEON


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)


class AmdSysfsGpuTest(unittest.TestCase):
    def setUp(self):
        self.root = Path(tempfile.mkdtemp())

    def add_card(self, name, busy, vram_used, vram_total, millidegrees=None):
        device = self.root / name / "device"
        write(device / "gpu_busy_percent", f"{busy}\n")
        write(device / "mem_info_vram_used", str(vram_used))
        write(device / "mem_info_vram_total", str(vram_total))
        if millidegrees is not None:
            write(device / "hwmon" / "hwmon3" / "temp1_input", str(millidegrees))

    def test_picks_card_with_most_vram_and_reads_it(self):
        self.add_card("card0", 5, 256 * 1024**2, 512 * 1024**2)
        self.add_card("card1", 64, 4 * BYTES_PER_GIB, 16 * BYTES_PER_GIB, millidegrees=61000)
        write(self.root / "card1-DP-1" / "status", "connected")
        reading = AmdSysfsGpu.detect(self.root).read()
        self.assertEqual(reading.load_percent, 64.0)
        self.assertEqual(reading.temperature_c, 61.0)
        self.assertAlmostEqual(reading.vram_percent, 25.0)

    def test_none_without_amd_cards(self):
        write(self.root / "card0" / "device" / "vendor", "0x8086")
        self.assertIsNone(AmdSysfsGpu.detect(self.root))


class AppleIoregGpuTest(unittest.TestCase):
    def test_apple_silicon_uses_unified_memory(self):
        source = AppleIoregGpu(run=lambda command: IOREG_APPLE_SILICON, system_memory=lambda: 16 * BYTES_PER_GIB)
        reading = source.read()
        self.assertEqual(reading.load_percent, 34.0)
        self.assertIsNone(reading.temperature_c)
        self.assertAlmostEqual(reading.vram_used_gb, 917504000 / BYTES_PER_GIB)
        self.assertAlmostEqual(reading.vram_total_gb, 16.0)

    def test_intel_mac_reports_busiest_gpu_and_dedicated_vram(self):
        reading = AppleIoregGpu(run=lambda command: IOREG_INTEL_MAC_WITH_RADEON).read()
        self.assertEqual(reading.load_percent, 57.0)
        self.assertAlmostEqual(reading.vram_used_gb, 1.0)
        self.assertAlmostEqual(reading.vram_total_gb, 4.0)

    def test_detect_fails_without_ioreg(self):
        self.assertIsNone(AppleIoregGpu.detect(run=lambda command: None))


class FakeNvml:
    NVML_TEMPERATURE_GPU = 0

    class NVMLError(Exception):
        pass

    class Utilization:
        gpu = 73

    class Memory:
        used = 3 * BYTES_PER_GIB
        total = 12 * BYTES_PER_GIB

    def __init__(self, device_count=1):
        self.device_count = device_count

    def nvmlInit(self):
        if self.device_count < 0:
            raise self.NVMLError("driver not loaded")

    def nvmlDeviceGetCount(self):
        return self.device_count

    def nvmlDeviceGetHandleByIndex(self, index):
        return f"handle-{index}"

    def nvmlDeviceGetName(self, handle):
        return b"NVIDIA GeForce RTX 4070"

    def nvmlDeviceGetUtilizationRates(self, handle):
        return self.Utilization()

    def nvmlDeviceGetTemperature(self, handle, sensor):
        return 66

    def nvmlDeviceGetMemoryInfo(self, handle):
        return self.Memory()


class NvidiaGpuTest(unittest.TestCase):
    def test_reads_load_temperature_and_vram(self):
        gpu = NvidiaGpu.detect(nvml=FakeNvml())
        self.assertEqual(gpu.description, "NVIDIA GeForce RTX 4070 (NVML)")
        reading = gpu.read()
        self.assertEqual((reading.load_percent, reading.temperature_c), (73.0, 66.0))
        self.assertAlmostEqual(reading.vram_percent, 25.0)

    def test_none_when_driver_missing(self):
        self.assertIsNone(NvidiaGpu.detect(nvml=FakeNvml(device_count=-1)))

    def test_none_when_index_out_of_range(self):
        self.assertIsNone(NvidiaGpu.detect(device_index=2, nvml=FakeNvml(device_count=1)))


if __name__ == "__main__":
    unittest.main()
