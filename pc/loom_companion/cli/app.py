"""Main program: picks a run mode and drives the collect-and-send loop."""

from __future__ import annotations

import time
from typing import List, Optional

from ..config import RECONNECT_DELAY_SECONDS
from ..core.collector import StatsCollector
from ..transport.device_link import DeviceLink
from ..transport.ports import list_serial_ports
from ..transport.protocol import encode
from . import console
from .arguments import parse_arguments


def sleep_until_next_update(started_at: float, interval_seconds: float) -> None:
    """Sleep for whatever is left of ``interval_seconds`` since ``started_at``.

    Args:
        started_at: ``time.monotonic()`` value taken at the start of the update.
        interval_seconds: Desired time between updates.
    """
    remaining = interval_seconds - (time.monotonic() - started_at)
    if remaining > 0:
        time.sleep(remaining)


def run_dry(collector: StatsCollector, interval_seconds: float) -> None:
    """Print a live status line forever without talking to a device."""
    while True:
        started_at = time.monotonic()
        console.rewrite_line(console.format_status(collector.collect()))
        sleep_until_next_update(started_at, interval_seconds)


def run_with_device(collector: StatsCollector, link: DeviceLink, interval_seconds: float) -> None:
    """Stream snapshots to the M5Stack forever, reconnecting whenever the link drops.

    While disconnected, a waiting message is shown and connection is retried every
    ``RECONNECT_DELAY_SECONDS``. Connect and disconnect transitions are announced once.
    """
    was_connected = False
    with link:
        while True:
            started_at = time.monotonic()
            snapshot = collector.collect()
            sent = link.send(encode(snapshot))
            if sent and not was_connected:
                console.announce(f"Connected to {link.port_name}. Press Ctrl+C to stop.")
            if not sent and was_connected:
                console.announce("Lost the M5Stack. Reconnecting...")
            was_connected = sent
            if not sent:
                console.rewrite_line("Waiting for the M5Stack... (plug it in, or pass --port)")
                time.sleep(RECONNECT_DELAY_SECONDS)
                continue
            console.rewrite_line(console.format_status(snapshot))
            sleep_until_next_update(started_at, interval_seconds)


def report_sensors(collector: StatsCollector) -> None:
    """Print a single status line after a short warm-up.

    The one-second delay lets interval-based measurements such as CPU load produce
    a meaningful first value.
    """
    time.sleep(1.0)
    print(console.format_status(collector.collect()))


def main(argv: Optional[List[str]] = None) -> int:
    """Run the companion.

    Args:
        argv: Command-line arguments, or ``None`` to use ``sys.argv``.

    Returns:
        Process exit code. Ctrl+C is treated as a normal stop and returns 0.
    """
    arguments = parse_arguments(argv)
    if arguments.list_ports:
        console.show_ports(list_serial_ports())
        return 0

    collector = StatsCollector(nvidia_device_index=arguments.gpu_index)
    console.show_sources(collector.describe_sources())
    try:
        if arguments.sensors:
            report_sensors(collector)
        elif arguments.dry_run:
            run_dry(collector, arguments.interval)
        else:
            run_with_device(collector, DeviceLink(arguments.port, arguments.baud), arguments.interval)
    except KeyboardInterrupt:
        console.announce("Stopped.")
    return 0
