"""Command-line argument definitions."""

from __future__ import annotations

import argparse
from typing import List, Optional

from ..config import DEFAULT_BAUD_RATE, DEFAULT_SEND_INTERVAL_SECONDS


def build_parser() -> argparse.ArgumentParser:
    """Build the parser; ``--list-ports``, ``--sensors`` and ``--dry-run`` are mutually exclusive."""
    parser = argparse.ArgumentParser(
        prog="temporal_loom",
        description="Send this computer's CPU, GPU and memory stats to the Temporal Loom M5Stack.",
    )
    parser.add_argument("--port", help="serial port of the M5Stack, e.g. COM5 or /dev/ttyUSB0 (auto-detected)")
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD_RATE, help="serial speed (default %(default)s)")
    parser.add_argument(
        "--interval", type=float, default=DEFAULT_SEND_INTERVAL_SECONDS, help="seconds between updates (default %(default)s)"
    )
    parser.add_argument("--gpu-index", type=int, default=0, help="which NVIDIA GPU to watch on multi-GPU systems")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--list-ports", action="store_true", help="list serial ports and exit")
    modes.add_argument("--sensors", action="store_true", help="show detected sensors and one reading, then exit")
    modes.add_argument("--dry-run", action="store_true", help="print stats without an M5Stack connected")
    return parser


def parse_arguments(argv: Optional[List[str]] = None) -> argparse.Namespace:
    """Parse ``argv`` (or ``sys.argv`` when ``None``) into a namespace."""
    return build_parser().parse_args(argv)
