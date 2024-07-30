# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from itertools import product
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "extravars"
    description = "Verifies that extra variables start with an underscore"

    def analyze(self, pkginfo, tar):
        carch_vars = [
            "checkdepends",
            "conflicts",
            "depends",
            "makedepends",
            "optdepends",
            "provides",
            "replaces",
            "source",
            "md5sums",
            "sha224sums",
            "sha1sums",
            "sha256sums",
            "sha384sums",
            "sha512sums",
            "b2sums",
        ]
        stdvars = [
            "arch",
            "license",
            "backup",
            "noextract",
            "pkgname",
            "pkgbase",
            "pkgver",
            "pkgrel",
            "epoch",
            "pkgdesc",
            "groups",
            "url",
            "install",
            "changelog",
            "options",
            "validpgpkeys",
        ] + carch_vars
        if "arch" in pkginfo and pkginfo["arch"] != ["any"]:
            stdvars.extend(v + "_" + a for v, a in product(carch_vars, pkginfo["arch"]))
        for varname in pkginfo["setvars"]:
            if varname.islower() and varname not in stdvars and not varname.startswith("_"):
                self.warnings.append(("extra-var-begins-without-underscore %s", varname))
