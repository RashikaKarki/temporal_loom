"""Finds the process currently using the most CPU."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict

import psutil

from ...config import MAX_PROCESS_NAME_LENGTH

SYSTEM_IDLE_PID = 0


@dataclass(frozen=True)
class TopProcess:
    """The busiest process.

    Attributes:
        name: Process name, truncated to ``MAX_PROCESS_NAME_LENGTH``, or ``--`` if none.
        percent: Share of total CPU capacity, 0-100.
    """
    name: str = "--"
    percent: float = 0.0


class TopProcessTracker:
    """Tracks per-process CPU usage between samples to find the busiest process.

    psutil measures process CPU usage between calls, so processes are kept across
    samples. The constructor takes an initial sample to prime the measurements.
    """
    def __init__(self):
        self._tracked: Dict[int, psutil.Process] = {}
        self._logical_cpus = psutil.cpu_count() or 1
        self.sample()

    def sample(self) -> TopProcess:
        """Return the process with the highest CPU share since the previous sample.

        The system idle process is ignored. Processes seen for the first time are only
        primed and count from the next sample.
        """
        busiest = TopProcess()
        alive = set()
        for process in psutil.process_iter(["name"]):
            if process.pid == SYSTEM_IDLE_PID:
                continue
            alive.add(process.pid)
            share = self._cpu_share(process)
            if share is not None and share > busiest.percent:
                name = (process.info.get("name") or "?")[:MAX_PROCESS_NAME_LENGTH]
                busiest = TopProcess(name, share)
        self._forget_exited(alive)
        return busiest

    def _cpu_share(self, process: psutil.Process):
        """Return the process's share of total CPU capacity, or ``None`` if newly seen or inaccessible.

        psutil reports per-process usage relative to one core, so it is divided by the
        logical CPU count.
        """
        tracked = self._tracked.get(process.pid)
        try:
            if tracked is None:
                self._tracked[process.pid] = process
                process.cpu_percent(None)
                return None
            return tracked.cpu_percent(None) / self._logical_cpus
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            return None

    def _forget_exited(self, alive: set) -> None:
        """Stop tracking processes that are no longer running."""
        for pid in list(self._tracked):
            if pid not in alive:
                del self._tracked[pid]
