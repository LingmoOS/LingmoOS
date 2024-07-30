# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.ruleclass import TarballRule


class systemdlocationRule(TarballRule):
    name = "systemdlocation"
    description = "Checks for systemd files in /etc/systemd/system/"

    def analyze(self, pkginfo, tar):
        # don't have this warning for the systemd package
        if "name" in pkginfo:
            if pkginfo["name"] == "systemd":
                return
        # don't have this warning for packages that provides systemd
        if "provides" in pkginfo:
            if "systemd" in pkginfo["provides"]:
                return
        for entry in tar:
            # ignore the actual directory, as that's handled by emptydirs
            if entry.isdir():
                continue

            name = os.path.normpath(entry.name)

            # check for files in /etc/systemd/system/
            if name.startswith("etc/systemd/system/"):
                self.warnings.append(("systemd-location %s", name))
