"""CPU temperature on macOS from the ``smctemp`` or ``osx-cpu-temp`` command-line tools."""

from __future__ import annotations

import re
from typing import Callable, Optional, Sequence

from ..readings import CpuTemperatureSource
from ..system.commands import is_installed, run_command

TEMPERATURE_COMMANDS = (("smctemp", "-c"), ("osx-cpu-temp",))
NUMBER_PATTERN = re.compile(r"-?\d+(?:\.\d+)?")

CommandRunner = Callable[[Sequence[str]], Optional[str]]


class SmcCommandCpuTemperature(CpuTemperatureSource):
    """Reads CPU temperature by running an SMC command and parsing the first number it prints.

    Args:
        command: Program and arguments to run.
        run: Command runner returning stdout or ``None``, injectable for tests.
    """
    def __init__(self, command: Sequence[str], run: CommandRunner = run_command):
        self._command = tuple(command)
        self._run = run
        self.description = f"{command[0]} command"

    @classmethod
    def detect(
        cls, run: CommandRunner = run_command, installed: Callable[[str], bool] = is_installed
    ) -> Optional["SmcCommandCpuTemperature"]:
        """Return a source for the first installed tool that produces a reading, otherwise ``None``."""
        for command in TEMPERATURE_COMMANDS:
            if installed(command[0]):
                source = cls(command, run)
                if source.read() is not None:
                    return source
        return None

    def read(self) -> Optional[float]:
        """Return the temperature in °C, or ``None`` if the command fails or reports a non-positive value."""
        output = self._run(self._command)
        match = NUMBER_PATTERN.search(output or "")
        if not match:
            return None
        value = float(match.group())
        return value if value > 0 else None
