"""Overall CPU load via psutil."""

import psutil


class CpuLoadMeter:
    """Measures overall CPU load as a percentage since the previous read.

    The constructor primes psutil so the first ``read`` returns a real value rather than 0.
    """
    def __init__(self):
        psutil.cpu_percent(None)

    def read(self) -> float:
        """Return CPU load, 0-100, averaged over the time since the last call."""
        return psutil.cpu_percent(None)
