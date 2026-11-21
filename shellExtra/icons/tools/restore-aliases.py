#!/usr/bin/env python3
"""Restore flattened icon symlinks after the artwork and alias trees merge."""
from pathlib import Path
import re
import sys


def restore(theme):
    theme = Path(theme).resolve()
    restored = 0
    for path in sorted(theme.rglob("*")):
        if path.is_symlink() or not path.is_file() or path.suffix not in (".svg", ".png", ".xpm"):
            continue
        if path.stat().st_size > 512:
            continue
        try:
            target = path.read_text().strip()
        except (UnicodeError, OSError):
            continue
        if not re.fullmatch(r"[A-Za-z0-9_.+@/ -]+\.(svg|png|xpm)", target):
            continue
        resolved = (path.parent / target).resolve()
        if not resolved.is_relative_to(theme):
            raise ValueError("Icon alias escapes theme: {} -> {}".format(path, target))
        if resolved == path:
            raise ValueError("Self-referencing icon alias: {}".format(path))
        path.unlink()
        path.symlink_to(target)
        restored += 1
    broken = [str(path.relative_to(theme)) for path in theme.rglob("*") if path.is_symlink() and not path.exists()]
    if broken:
        raise ValueError("Broken icon aliases: " + ", ".join(broken[:20]))
    return restored


if __name__ == "__main__":
    print("Restored {} icon aliases".format(restore(sys.argv[1])))
