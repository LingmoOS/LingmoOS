# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "emptydir"
    description = "Warns about empty directories in a package"

    def analyze(self, pkginfo, tar):
        dirs = []
        entries = []
        for entry in tar:
            if entry.isdir():
                dirs.append(entry.name.rstrip("/"))
            entries.append(entry.name.rstrip("/"))
        nonemptydirs = [os.path.dirname(x) for x in entries]
        self.warnings = [("empty-directory %s", d) for d in (set(dirs) - set(nonemptydirs))]
