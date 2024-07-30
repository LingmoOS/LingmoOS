# Copyright (C) 2024 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.ruleclass import TarballRule


class dbus1locationRule(TarballRule):
    name = "dbus1location"
    description = "Checks for dbus files in /etc/dbus1/system.d/"

    def analyze(self, _, tar):
        for entry in tar:
            # ignore the actual directory, as that's handled by emptydirs
            if entry.isdir():
                continue

            name = os.path.normpath(entry.name)

            # check for files in /etc/dbus-1/system.d/
            if name.startswith("etc/dbus-1/system.d/"):
                self.warnings.append(("dbus-1-location %s", name))
