# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"These rules checks basic sanity of package metadata"

import re
import os
from Namcap.ruleclass import PkgInfoRule, PkgbuildRule
from Namcap.util import is_debug


class CapsPkgnameRule(PkgInfoRule):
    name = "capsnamespkg"
    description = "Verifies package name in package does not include upper case letters"

    def analyze(self, pkginfo, tar):
        if re.search(r"[A-Z]", pkginfo["name"]) is not None:
            self.errors.append(("package-name-in-uppercase", ()))


class UrlRule(PkgInfoRule):
    name = "urlpkg"
    description = "Verifies url is included in a package file"

    def analyze(self, pkginfo, tar):
        if "url" not in pkginfo:
            self.errors.append(("missing-url", ()))


class LicenseRule(PkgInfoRule):
    name = "license"
    description = "Verifies license is included in a PKGBUILD"

    def analyze(self, pkginfo, tar):
        if is_debug(pkginfo):
            return
        if "license" not in pkginfo or len(pkginfo["license"]) == 0:
            self.errors.append(("missing-license", ()))


class NonUniqueSourcesRule(PkgbuildRule):
    name = "non-unique-source"
    description = "Verifies the downloaded sources have a unique filename"

    def analyze(self, pkginfo, tar):
        for source_file in pkginfo["source"]:
            if "::" not in source_file and re.match(
                r"^[vV]?(([0-9]){8}|([0-9]+\.?)+)\.", os.path.basename(source_file)
            ):
                self.warnings.append(("non-unique-source-name %s", os.path.basename(source_file)))
