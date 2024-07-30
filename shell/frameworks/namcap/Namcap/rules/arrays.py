# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Verifies that array variables are actually arrays"""

import re
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "array"
    description = "Verifies that array variables are actually arrays"

    def analyze(self, pkginfo, tar):
        arrayvars = [
            "arch",
            "license",
            "groups",
            "depends",
            "makedepends",
            "optdepends",
            "checkdepends",
            "provides",
            "conflicts",
            "replaces",
            "backup",
            "options",
            "source",
            "noextract",
            "md5sums",
            "sha1sums",
            "sha224sums",
            "sha256sums",
            "sha384sums",
            "sha512sums",
            "b2sums",
            "validpgpkeys",
        ]
        for i in pkginfo.pkgbuild:
            m = re.match(r"\s*(.*)\s*=\s*(.*)$", i)
            for j in arrayvars:
                if m and m.group(1) == j:
                    if not m.group(2).startswith("("):
                        self.warnings.append(("variable-not-array %s", (j,)))
