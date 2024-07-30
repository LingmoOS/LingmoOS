# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""
This contains a collection of essentially one-line rules:
If a certain path is detected then a certain dependency is expected.

Anything fancier than this should get its own rule.
"""

import re
from Namcap.ruleclass import TarballRule


class PathDependsRule(TarballRule):
    name = "pathdepends"
    description = "Check for simple implicit path dependencies"
    # list of path regex, dep name, reason tag
    subrules = [
        {"path": r"^usr/share/glib-2\.0/schemas$", "dep": "dconf", "reason": "dconf-needed-for-glib-schemas"},
        {"path": r"^usr/lib/gio/modules/.*\.so$", "dep": "glib2", "reason": "glib2-needed-for-gio-modules"},
        {
            "path": r"^usr/share/icons/hicolor$",
            "dep": "hicolor-icon-theme",
            "reason": "hicolor-icon-theme-needed-for-hicolor-dir",
        },
    ]

    def analyze(self, pkginfo, tar):
        names = [entry.name for entry in tar]
        for subrule in self.subrules:
            pattern = re.compile(subrule["path"])
            if any(pattern.search(n) for n in names):
                dep = subrule["dep"]
                reason = subrule["reason"]
                pkginfo.detected_deps[dep].append((reason, ()))
