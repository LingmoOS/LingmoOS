# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re
from Namcap.ruleclass import TarballRule


class HookDependsRule(TarballRule):
    name = "hookdepends"
    description = "Check for redundant hook dependencies"
    subrules = [
        {
            "path": r"^usr/share/applications/.*\.desktop$",
            "dep": "desktop-file-utils",
        },
        {
            "path": r"^usr/share/mime$",
            "dep": "shared-mime-info",
        },
    ]

    def analyze(self, pkginfo, tar):
        names = [entry.name for entry in tar]
        for subrule in self.subrules:
            dep = subrule["dep"]
            if dep not in pkginfo["depends"]:
                continue
            pattern = re.compile(subrule["path"])
            if any(pattern.search(n) for n in names):
                self.warnings.append(("external-hooks-unneeded %s", (dep,)))
