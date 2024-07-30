# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class InfodirRule(TarballRule):
    name = "infodirectory"
    description = "Checks for info directory file."

    def analyze(self, pkginfo, tar):
        for i in tar.getnames():
            if i == "usr/share/info/dir":
                self.errors.append(("info-dir-file-present %s", i))
