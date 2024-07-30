# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Checks for missing variables in PKGBUILD"""

import re
from Namcap.ruleclass import PkgbuildRule

RE_IS_HEXNUMBER = re.compile(r"[0-9a-f]+")


class ChecksumsRule(PkgbuildRule):
    name = "checksums"
    description = "Verifies checksums are included in a PKGBUILD"

    def analyze(self, pkginfo, tar):
        checksums = [
            ("md5", 32),
            ("sha1", 40),
            ("sha224", 56),
            ("sha256", 64),
            ("sha384", 96),
            ("sha512", 128),
            ("b2", 128),
        ]

        if "source" in pkginfo:
            haschecksums = False
            for sumname, sumlen in checksums:
                if (sumname + "sums") in pkginfo:
                    haschecksums = True
            if not haschecksums:
                self.errors.append(("missing-checksums", ()))
        else:
            pkginfo["source"] = []

        for sumname, sumlen in checksums:
            sumname += "sums"
            if sumname in pkginfo:
                if len(pkginfo["source"]) > len(pkginfo[sumname]):
                    self.errors.append(("not-enough-checksums %s %i needed", (sumname, len(pkginfo["source"]))))
                elif len(pkginfo["source"]) < len(pkginfo[sumname]):
                    self.errors.append(("too-many-checksums %s %i needed", (sumname, len(pkginfo["source"]))))
                for csum in pkginfo[sumname]:
                    if csum == "SKIP":
                        continue
                    if len(csum) != sumlen or not RE_IS_HEXNUMBER.match(csum):
                        self.errors.append(("improper-checksum %s %s", (sumname, csum)))


class TagsRule(PkgbuildRule):
    name = "tags"
    description = "Looks for Maintainer and Contributor comments"

    def analyze(self, pkginfo, tar):
        contributortag = 0
        maintainertag = 0
        for i in pkginfo.pkgbuild:
            if re.match(r"#\s*Contributor\s*:", i):
                contributortag = 1
            if re.match(r"#\s*Maintainer\s*:", i):
                maintainertag = 1

        if contributortag != 1:
            self.infos.append(("missing-contributor", ()))

        if maintainertag != 1:
            self.warnings.append(("missing-maintainer", ()))


class DescriptionRule(PkgbuildRule):
    name = "description"
    description = "Verifies that the description is set in a PKGBUILD"

    def analyze(self, pkginfo, tar):
        if pkginfo.is_split:
            for split_pkginfo in pkginfo.subpackages:
                if "desc" not in split_pkginfo or len(split_pkginfo["desc"]) == 0:
                    self.errors.append(("missing-description %s", (split_pkginfo["name"],)))
        else:
            if "desc" not in pkginfo or len(pkginfo["desc"]) == 0:
                self.errors.append(("missing-description", ()))
