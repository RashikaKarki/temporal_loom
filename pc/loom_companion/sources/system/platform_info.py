"""Operating system checks."""

import sys


def is_windows() -> bool:
    """Return whether running on Windows."""
    return sys.platform.startswith("win")


def is_macos() -> bool:
    """Return whether running on macOS."""
    return sys.platform == "darwin"


def is_linux() -> bool:
    """Return whether running on Linux."""
    return sys.platform.startswith("linux")
