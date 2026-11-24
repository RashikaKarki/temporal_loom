"""Command-line entry point for the Temporal Loom PC companion.

Run ``python temporal_loom.py --help`` for options.
"""

from loom_companion.cli.app import main

if __name__ == "__main__":
    raise SystemExit(main())
