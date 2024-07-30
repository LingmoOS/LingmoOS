# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "invalidstartdir"
    description = "Looks for references to $startdir"

    def analyze(self, pkginfo, tar):
        for i in pkginfo.pkgbuild:
            startdirs = re.split(r'\${?startdir}?"?', i)
            for j in startdirs[1:]:
                if j[:4] != "/pkg" and j[:4] != "/src":
                    self.errors.append(("file-referred-in-startdir", ()))
                elif j[:4] == "/pkg":
                    self.errors.append(("use-pkgdir", ()))
                elif j[:4] == "/src":
                    self.errors.append(("use-srcdir", ()))
