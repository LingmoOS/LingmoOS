# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Checks for bad backup entries"""

import re
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "badbackups"
    description = "Checks for bad backup entries"

    def analyze(self, pkginfo, tar):
        if "backup" in pkginfo:
            for item in pkginfo["backup"]:
                if re.match(r"^/", item) is not None:
                    self.errors.append(("backups-preceding-slashes", ()))
