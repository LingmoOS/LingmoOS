# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Check for py timestamps that are ahead of pyc/pyo timestamps
"""

import os
from Namcap.package import load_mtree
from Namcap.ruleclass import TarballRule


def _quick_filter(names):
    "can this package be skipped outright"
    if not names:
        return True
    found_py = any(n.endswith(".py") for n in names)
    found_pyc = any(n.endswith(".pyc") for n in names)
    found_pyo = any(n.endswith(".pyo") for n in names)
    if found_py and found_pyc:
        return False
    if found_py and found_pyo:
        return False
    return True


def _tar_timestamps(tar):
    "takes a tar object"
    return dict((m.name, m.mtime) for m in tar.getmembers())


def _mtree_timestamps(tar):
    "takes a tar object"
    return dict((h, a["time"]) for h, a in load_mtree(tar) if "time" in a)


def _generic_timestamps(tar):
    "works for mtree and tar"
    if ".MTREE" in tar.getnames():
        return _mtree_timestamps(tar)
    return _tar_timestamps(tar)


def _try_mtree(tar):
    "returns True if good, False if bad, None if N/A"
    if ".MTREE" not in tar.getnames():
        return None
    stamps = _mtree_timestamps(tar)
    if _quick_filter(stamps.keys()):
        return True
    return not _mtime_filter(stamps)


def _try_tar(tar):
    "returns True if good, False if bad"
    names = tar.getnames()
    if _quick_filter(names):
        return True
    mtimes = _tar_timestamps(tar)
    return not _mtime_filter(mtimes)


def _split_all(path):
    "like os.path.split but splits every directory"
    p2 = path
    dirs: list[str] = []
    while p2 and p2 != "/":
        p2, p3 = os.path.split(p2)
        dirs.insert(0, p3)
    # dirs.insert(0, '/')
    return dirs


def _source_py(path):
    "given a pyc/pyo, return the source path"
    if not path.endswith(".pyc") and not path.endswith(".pyo"):
        return None
    path = path[:-1]
    # handle py2
    if "__pycache__" not in path:
        return path
    # handle py3
    splitup = _split_all(path)
    if splitup[-2] != "__pycache__":
        return None
    splitup.pop(-2)
    f = splitup[-1]
    f = f.split(".")
    f.pop(-2)
    splitup[-1] = ".".join(f)
    return os.path.join(*splitup)


def _mtime_filter(mtimes):
    "return list of bad py file names"
    bad = []
    for name, mt2 in mtimes.items():
        if not name.endswith(".pyc") and not name.endswith(".pyo"):
            continue
        source_name = _source_py(name)
        if source_name not in mtimes:
            continue
        mt1 = mtimes[source_name]
        if mt1 > mt2:
            bad.append(source_name)
    return bad


class package(TarballRule):
    name = "py_mtime"
    description = "Check for py timestamps that are ahead of pyc/pyo timestamps"

    def analyze(self, pkginfo, tar):
        mtree_status = _try_mtree(tar)
        tar_status = _try_tar(tar)
        if mtree_status is False and tar_status:
            # mtree only
            self.warnings = [("py-mtime-mtree-warning", ())]
        elif not tar_status:
            # tar or both
            self.errors = [("py-mtime-tar-error", ())]
        self.infos = [("py-mtime-file-name %s", f[1:]) for f in _mtime_filter(_generic_timestamps(tar))]
