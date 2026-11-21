#!/usr/bin/env python3
"""Read the exact canonical version already validated by lingmo-version check."""
import json
import re
import sys


def build_version(document):
    value = document["source"]["build_ver"]
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*", value) or ".." in value:
        raise ValueError("Invalid canonical build version")
    return value


if __name__ == "__main__":
    with open(sys.argv[1]) as stream:
        print("build-version=" + build_version(json.load(stream)))
