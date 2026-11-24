"""Helpers for running external command-line tools safely."""

from __future__ import annotations

import shutil
import subprocess
from typing import Optional, Sequence


def is_installed(program: str) -> bool:
    """Return whether ``program`` is found on ``PATH``."""
    return shutil.which(program) is not None


def run_command(arguments: Sequence[str], timeout_seconds: float = 1.0) -> Optional[str]:
    """Run a command and return its stdout.

    Returns:
        Standard output, or ``None`` if the program is missing, fails to start, times
        out or exits with a non-zero status.
    """
    if not is_installed(arguments[0]):
        return None
    try:
        completed = subprocess.run(
            list(arguments), capture_output=True, text=True, timeout=timeout_seconds, check=False
        )
    except (OSError, subprocess.SubprocessError):
        return None
    return completed.stdout if completed.returncode == 0 else None
