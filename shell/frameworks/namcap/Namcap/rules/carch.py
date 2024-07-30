# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Verifies that no specific host type is used"""

import re
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "carch"
    description = "Verifies that no specific host type is used"

    def analyze(self, pkginfo, tar):
        arches = ["arm", "i586", "i686", "ppc", "x86_64"]
        archmatch = re.compile(r"\b(%s)\b" % "|".join(arches))
        # Match an arch=(i686) line
        archline = re.compile(r"\w*arch=")
        for line in pkginfo.pkgbuild:
            # strip any comments
            line = line.split("#")[0]
            match = archmatch.search(line)
            if not match:
                continue
            if archline.match(line):
                continue
            if "$CARCH" in line:
                continue
            if "${CARCH}" in line:
                continue
            self.warnings.append(("specific-host-type-used %s", (match.group(1),)))
