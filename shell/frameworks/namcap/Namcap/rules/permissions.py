# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import stat
from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "permissions"
    description = "Checks file permissions."

    def analyze(self, pkginfo, tar):
        for i in tar.getmembers():
            if not i.mode & stat.S_IROTH and not (i.issym() or i.islnk()):
                self.warnings.append(("file-not-world-readable %s", i.name))
            if i.mode & stat.S_IWOTH and not (i.issym() or i.islnk()):
                self.warnings.append(("file-world-writable %s", i.name))
            if not i.mode & stat.S_IXOTH and i.isdir():
                self.warnings.append(("directory-not-world-executable %s", i.name))
            if str(i.name).endswith(".a") and not (i.issym() or i.islnk()):
                if i.mode != 0o644 and i.mode != 0o444:
                    self.warnings.append(("incorrect-library-permissions %s", i.name))
            if i.mode & (stat.S_ISUID | stat.S_ISGID):
                self.warnings.append(("file-setugid %s", i.name))
