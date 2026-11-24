"""Client for LibreHardwareMonitor's web server JSON (Windows).

LibreHardwareMonitor must be running with its remote web server enabled on
``LIBRE_HARDWARE_MONITOR_URL``.
"""

from __future__ import annotations

import json
import re
import time
import urllib.request
from dataclasses import dataclass
from typing import Callable, Iterator, List, Optional

from ...config import (
    LIBRE_HARDWARE_MONITOR_CACHE_SECONDS,
    LIBRE_HARDWARE_MONITOR_RETRY_SECONDS,
    LIBRE_HARDWARE_MONITOR_TIMEOUT_SECONDS,
    LIBRE_HARDWARE_MONITOR_URL,
)

VALUE_PATTERN = re.compile(r"^\s*(-?[\d.,]+)\s*(.*?)\s*$")


@dataclass(frozen=True)
class HardwareSensor:
    """One sensor value from the LibreHardwareMonitor tree.

    Attributes:
        hardware: Hardware device name, e.g. a CPU or GPU model.
        group: Sensor group such as ``Temperatures``, ``Load`` or ``SmallData``.
        name: Sensor name.
        value: Numeric value.
        unit: Unit text such as ``°C``, ``%`` or ``MB``.
    """
    hardware: str
    group: str
    name: str
    value: float
    unit: str


def parse_sensor_value(text: object) -> Optional[tuple]:
    """Split a value string like ``"61.5 °C"`` into ``(number, unit)``.

    Accepts either a comma or a dot as the decimal separator; when both appear,
    commas are treated as thousands separators.

    Returns:
        ``(value, unit)``, or ``None`` if the text does not start with a number.
    """
    match = VALUE_PATTERN.match(str(text or ""))
    if not match:
        return None
    number, unit = match.groups()
    if "," in number and "." in number:
        number = number.replace(",", "")
    else:
        number = number.replace(",", ".")
    try:
        return float(number), unit
    except ValueError:
        return None


def flatten_sensor_tree(node: dict, parent: Optional[dict] = None, grandparent: Optional[dict] = None) -> Iterator[HardwareSensor]:
    """Yield every leaf sensor in the tree with its hardware and group names.

    The hardware is taken from the leaf's grandparent node and the group from its
    parent. Leaves without a numeric value are skipped.
    """
    children = node.get("Children") or []
    if children:
        for child in children:
            yield from flatten_sensor_tree(child, node, parent)
        return
    parsed = parse_sensor_value(node.get("Value"))
    if parsed is None or parent is None or grandparent is None:
        return
    value, unit = parsed
    yield HardwareSensor(
        hardware=str(grandparent.get("Text", "")),
        group=str(parent.get("Text", "")),
        name=str(node.get("Text", "")),
        value=value,
        unit=unit,
    )


def fetch_json(url: str, timeout_seconds: float) -> Optional[dict]:
    """Fetch and decode JSON from ``url``, or return ``None`` on any network or parse error."""
    try:
        with urllib.request.urlopen(url, timeout=timeout_seconds) as response:
            return json.load(response)
    except (OSError, ValueError):
        return None


class LibreHardwareMonitor:
    """Cached access to LibreHardwareMonitor's sensors.

    Successful fetches are cached for ``LIBRE_HARDWARE_MONITOR_CACHE_SECONDS``. When the
    server is unreachable, an empty list is returned and the next attempt waits
    ``LIBRE_HARDWARE_MONITOR_RETRY_SECONDS`` so a missing server does not slow every update.

    Args:
        url: Address of the ``data.json`` endpoint.
        fetch: JSON fetch function, injectable for tests.
        clock: Monotonic time function, injectable for tests.
    """
    def __init__(
        self,
        url: str = LIBRE_HARDWARE_MONITOR_URL,
        fetch: Callable[[str, float], Optional[dict]] = fetch_json,
        clock: Callable[[], float] = time.monotonic,
    ):
        self._url = url
        self._fetch = fetch
        self._clock = clock
        self._sensors: List[HardwareSensor] = []
        self._next_fetch_at = 0.0

    def sensors(self) -> List[HardwareSensor]:
        """Return all sensors, fetching fresh data when the cache or retry delay has expired."""
        now = self._clock()
        if now < self._next_fetch_at:
            return self._sensors
        tree = self._fetch(self._url, LIBRE_HARDWARE_MONITOR_TIMEOUT_SECONDS)
        if tree is None:
            self._sensors = []
            self._next_fetch_at = now + LIBRE_HARDWARE_MONITOR_RETRY_SECONDS
        else:
            self._sensors = list(flatten_sensor_tree(tree))
            self._next_fetch_at = now + LIBRE_HARDWARE_MONITOR_CACHE_SECONDS
        return self._sensors

    def is_running(self) -> bool:
        """Return whether LibreHardwareMonitor currently reports any sensors."""
        return bool(self.sensors())
