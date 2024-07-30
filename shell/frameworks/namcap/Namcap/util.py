# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re


def _file_has_magic(fileobj, magic_bytes):
    length = len(magic_bytes)
    magic = fileobj.read(length)
    fileobj.seek(0)
    return magic == magic_bytes


def is_elf(fileobj):
    "Take file object, peek at the magic bytes to check if ELF file."
    return _file_has_magic(fileobj, b"\x7fELF")


def is_static(fileobj):
    "Take file object, peek at the magic bytes to check if static lib."
    return _file_has_magic(fileobj, b"!<arch>\n")


def is_script(fileobj):
    "Take file object, peek at the magic bytes to check if script."
    return _file_has_magic(fileobj, b"#!")


def is_java(fileobj):
    "Take file object, peek at the magic bytes to check if class file."
    return _file_has_magic(fileobj, b"\xca\xfe\xba\xbe")


def script_type(fileobj):
    firstline = fileobj.readline()
    fileobj.seek(0)
    try:
        firstline = firstline.decode("utf-8", "strict")
    except UnicodeDecodeError:
        return None
    if not firstline:
        return None
    script = re.compile(r"#!.*/(.+)")
    m = script.match(firstline)
    if m is None:
        return None
    cmd = m.group(1).split()
    name = cmd[0]
    if name == "env":
        name = cmd[1]
    return name


def is_debug(pkginfo):
    "Take pkginfo, checks if it's a debug package"
    return "pkgdesc" in pkginfo and pkginfo["pkgdesc"].startswith("Detached debugging symbols for ")
