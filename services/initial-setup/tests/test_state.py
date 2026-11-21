#!/usr/bin/env python3
import configparser
import tempfile
from pathlib import Path


def mark(home: Path) -> None:
    path = home / ".config" / "lingmo" / "oobe.conf"
    path.parent.mkdir(parents=True)
    path.write_text("[Setup]\nComplete=true\n")


with tempfile.TemporaryDirectory() as directory:
    home = Path(directory)
    assert not (home / ".config/lingmo/oobe.conf").exists()
    mark(home)
    parser = configparser.ConfigParser()
    parser.read(home / ".config/lingmo/oobe.conf")
    assert parser.getboolean("Setup", "Complete") is True

print("OOBE state test passed")
