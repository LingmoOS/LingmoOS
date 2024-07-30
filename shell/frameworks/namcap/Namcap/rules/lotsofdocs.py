# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "lots-of-docs"
    description = "See if a package is carrying more documentation than it should"

    def analyze(self, pkginfo, tar):
        if "name" in pkginfo and (pkginfo["name"].endswith("-doc") or pkginfo["name"].endswith("-docs")):
            # Don't do anything if the package is called "*-doc"
            return
        docdir = "usr/share/doc"
        size = 0
        docsize = 0

        for i in tar.getmembers():
            if i.name.startswith(docdir):
                docsize += i.size
            size += i.size

        if size > 0:
            ratio = docsize / float(size)
            if ratio > 0.50:
                self.warnings.append(("lots-of-docs %f", (ratio * 100,)))
