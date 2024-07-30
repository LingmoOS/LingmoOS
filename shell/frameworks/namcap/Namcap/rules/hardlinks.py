# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from os.path import dirname
from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "hardlinks"
    description = "Look for cross-directory/partition hard links"

    def analyze(self, pkginfo, tar):
        hardlinks = [i for i in tar.getmembers() if i.islnk() is True]
        for hardlink in hardlinks:
            if dirname(hardlink.name) != dirname(hardlink.linkname):
                self.errors.append(("cross-dir-hardlink %s %s", (hardlink.name, hardlink.linkname)))
