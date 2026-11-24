import json
import re
import unittest
from pathlib import Path

from loom_companion.core.snapshot import Snapshot
from loom_companion.sources.readings import GpuReading
from loom_companion.sources.system.memory import MemoryUsage
from loom_companion.sources.system.processes import TopProcess
from loom_companion.transport.ports import SerialPortInfo, choose_device_port
from loom_companion.transport.protocol import encode, to_message

FIRMWARE_STATS_SOURCE = (
    Path(__file__).resolve().parents[2] / "firmware" / "temporal_loom" / "src" / "core" / "stats_store.cpp"
)


def sample_snapshot(cpu_temperature=55.4, gpu_temperature=None):
    return Snapshot(
        cpu_load_percent=41.6,
        cpu_temperature_c=cpu_temperature,
        gpu=GpuReading(load_percent=80.2, temperature_c=gpu_temperature, vram_used_gb=3.04, vram_total_gb=8.0),
        memory=MemoryUsage(percent=62.0, used_gb=9.94, total_gb=15.9),
        top_process=TopProcess("game.exe", 33.3),
        clock="21:07",
    )


class ProtocolTest(unittest.TestCase):
    def test_encodes_one_json_line(self):
        payload = encode(sample_snapshot())
        self.assertTrue(payload.endswith(b"\n"))
        self.assertEqual(payload.count(b"\n"), 1)
        self.assertEqual(json.loads(payload)["proc"], "game.exe")

    def test_unknown_temperature_is_minus_one(self):
        message = to_message(sample_snapshot(cpu_temperature=None))
        self.assertEqual(message["ctmp"], -1)
        self.assertEqual(message["gtmp"], -1)

    def test_missing_gpu_is_marked_unavailable(self):
        snapshot = sample_snapshot()
        message = to_message(Snapshot(**{**snapshot.__dict__, "gpu": None}))
        self.assertEqual((message["gpu"], message["gtmp"], message["vram"]), (-1, -1, -1))
        self.assertEqual((message["vramu"], message["vramt"]), (0, 0))

    def test_rounds_values(self):
        message = to_message(sample_snapshot())
        self.assertEqual((message["cpu"], message["ctmp"], message["vram"], message["vramu"]), (42, 55, 38, 3.0))

    def test_keys_match_what_firmware_reads(self):
        firmware_keys = set(re.findall(r'message\["(\w+)"\]', FIRMWARE_STATS_SOURCE.read_text()))
        self.assertEqual(set(to_message(sample_snapshot())), firmware_keys)

    def test_fits_firmware_line_buffer(self):
        self.assertLess(len(encode(sample_snapshot())), 384)


class ChooseDevicePortTest(unittest.TestCase):
    def test_prefers_known_usb_chip(self):
        ports = [
            SerialPortInfo("COM1", "Communications Port", None),
            SerialPortInfo("COM7", "USB-Enhanced-SERIAL CH9102", 0x1A86),
        ]
        self.assertEqual(choose_device_port(ports), "COM7")

    def test_prefers_macos_callout_device(self):
        ports = [
            SerialPortInfo("/dev/tty.usbserial-0001", "CP2104", 0x10C4),
            SerialPortInfo("/dev/cu.usbserial-0001", "CP2104", 0x10C4),
        ]
        self.assertEqual(choose_device_port(ports), "/dev/cu.usbserial-0001")

    def test_falls_back_to_description(self):
        ports = [SerialPortInfo("/dev/ttyUSB0", "CP2102 USB to UART Bridge", None)]
        self.assertEqual(choose_device_port(ports), "/dev/ttyUSB0")

    def test_none_when_nothing_matches(self):
        self.assertIsNone(choose_device_port([SerialPortInfo("/dev/ttyS0", "ttyS0", None)]))


if __name__ == "__main__":
    unittest.main()
