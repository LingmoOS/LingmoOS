# Copyright (C) 2024 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.ruleclass import TarballRule


class sphinxbuildcachefilesRule(TarballRule):
    name = "sphinxbuildcachefiles"
    description = "Checks for leftover sphinx-build cached environment files"

    def analyze(self, _pkginfo, tar):
        for entry in tar:
            if entry.isdir():
                continue

            name = os.path.normpath(entry.name)
            if name.endswith("environment.pickle"):
                self.errors.append(("sphinx-build-cache-files", ()))
