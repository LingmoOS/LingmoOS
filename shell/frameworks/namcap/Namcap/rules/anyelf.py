# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Check for ELF files to see if a package should be 'any' architecture
"""

import re
from Namcap.util import is_elf, is_static
from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "anyelf"
    description = "Check for ELF files to see if a package should be 'any' architecture"

    def analyze(self, pkginfo, tar):
        supress_name = ["^mingw-"]
        if any(re.search(s, pkginfo["name"]) for s in supress_name):
            return
        found_elffiles = []

        for entry in tar.getmembers():
            if not entry.isfile():
                continue
            f = tar.extractfile(entry)
            # Ar files (static libs) are also architecture specific (FS#24854)
            if is_elf(f) or is_static(f):
                found_elffiles.append(entry.name)
            f.close()

        if pkginfo["arch"] and pkginfo["arch"][0] == "any":
            self.errors = [("elffile-in-any-package %s", i) for i in found_elffiles]
        else:
            if len(found_elffiles) == 0:
                self.warnings.append(("no-elffiles-not-any-package", ()))
