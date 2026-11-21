#!/usr/bin/env python3
"""Wait for Lingmo Xwayland readiness and exec an X11 application."""

import argparse
import ctypes
import json
import os
from pathlib import Path
import re
import select
import sys
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ready-file", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=15)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        parser.error("a command is required after --")
    libc = ctypes.CDLL(None, use_errno=True)
    fd = libc.inotify_init1(os.O_CLOEXEC | os.O_NONBLOCK)
    if fd < 0:
        raise OSError(ctypes.get_errno(), "inotify_init1")
    try:
        if libc.inotify_add_watch(fd, os.fsencode(args.ready_file.parent), 0x80 | 0x8) < 0:
            raise OSError(ctypes.get_errno(), "cannot watch readiness directory")
        deadline = time.monotonic() + args.timeout
        while True:
            try:
                metadata = json.loads(args.ready_file.read_text())
                break
            except FileNotFoundError:
                remaining = deadline - time.monotonic()
                if remaining <= 0 or not select.select([fd], [], [], remaining)[0]:
                    raise TimeoutError("Xwayland did not publish readiness")
                os.read(fd, 65536)
    finally:
        os.close(fd)
    display = metadata.get("DISPLAY", "")
    authority = metadata.get("XAUTHORITY", "")
    if not re.fullmatch(r":\d+", display) or not Path(authority).is_file():
        raise ValueError("invalid or stale Xwayland readiness metadata")
    environment = dict(os.environ, DISPLAY=display, XAUTHORITY=authority)
    os.execvpe(command[0], command, environment)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, TimeoutError) as error:
        sys.exit(f"xwayland-launch: {error}")
