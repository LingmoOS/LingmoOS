# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import PkgInfoRule


class package(PkgInfoRule):
    name = "pkgnameindesc"
    description = "Verifies if the package name is included on package description"

    def analyze(self, pkginfo, tar):
        if "name" in pkginfo and "desc" in pkginfo:
            if pkginfo["name"].lower() in pkginfo["desc"].lower().split():
                self.warnings.append(("pkgname-in-description", ()))
