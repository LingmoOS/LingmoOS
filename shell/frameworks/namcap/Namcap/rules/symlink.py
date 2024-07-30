# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.ruleclass import TarballRule
from Namcap.package import load_from_db
from Namcap.util import is_debug


class package(TarballRule):
    name = "symlink"
    description = "Checks that symlinks point to the right place"

    def analyze(self, pkginfo, tar):
        filenames = set(s.name.rstrip("/") for s in tar)
        depfilenames = set()
        for d in pkginfo["depends"]:
            p = load_from_db(d)
            if not p:
                continue
            depfilenames |= set(name.rstrip("/") for name, _, _ in p["files"])
        # debug package needs the corresponding binary packages
        if is_debug(pkginfo):
            for d in [pkginfo["name"]] + pkginfo["provides"]:
                p = load_from_db(d[: -len("-debug")])
                if not p:
                    continue
                depfilenames |= set(name.rstrip("/") for name, _, _ in p["files"])
        filenames |= depfilenames
        for i in tar:
            if i.issym():
                self.infos.append(("symlink-found %s points to %s", (i.name, i.linkname)))
                linktarget = i.linkname
                linklead = os.path.dirname(i.name)
                # os.path.join drops the 1st arg if the 2nd one is absolute
                linkdest = os.path.join(linklead, linktarget)
                linkdest = os.path.normpath(linkdest).lstrip("/").rstrip("/")
                if linkdest not in filenames:
                    self.errors.append(("dangling-symlink %s points to %s", (i.name, i.linkname)))
            if i.islnk():
                self.infos.append(("hardlink-found %s points to %s", (i.name, i.linkname)))
                if i.linkname not in filenames:
                    self.errors.append(("dangling-hardlink %s points to %s", (i.name, i.linkname)))
