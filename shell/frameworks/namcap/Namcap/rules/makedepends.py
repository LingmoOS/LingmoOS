# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import PkgbuildRule


class RedundantMakedepends(PkgbuildRule):
    """
    This rule checks for make dependencies that are already
    included as dependencies.
    """

    name = "redundant_makedepends"
    description = "Check for redundant make dependencies"

    def analyze(self, pkginfo, pkgbuild):
        redundant_makedeps: list[str] = []

        if "makedepends" not in pkginfo:
            return
        if "depends" not in pkginfo:
            return
        redundant_makedeps.extend(set(pkginfo["makedepends"]) & set(pkginfo["depends"]))

        for i in redundant_makedeps:
            self.warnings.append(("redundant-makedep %s", (i,)))


class VCSMakedepends(PkgbuildRule):
    """
    This rule checks for missing VCS make dependencies.
    """

    name = "vcs_makedepends"
    description = "Verify make dependencies for VCS sources"

    def analyze(self, pkginfo, pkgbuild):
        vcs = {
            "bzr": "breezy",
            "git": "git",
            "hg": "mercurial",
            "svn": "subversion",
        }
        missing = []
        protocols = set()

        if "source" not in pkginfo:
            return

        for s in pkginfo["source"]:
            p = s.split("::", 1)[-1]
            p = p.split("://", 1)[0]
            p = p.split("+", 1)[0]
            if p in vcs:
                protocols.add(p)

        if not protocols:
            return

        for v in protocols:
            d = vcs[v]
            if "makedepends" not in pkginfo:
                missing.append(d)
                continue
            if d not in pkginfo["makedepends"]:
                missing.append(d)

        for i in missing:
            self.warnings.append(("missing-vcs-makedeps %s", (i,)))
