# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re
from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "scrollkeeper"
    description = "Verifies that there aren't any scrollkeeper directories."

    def analyze(self, pkginfo, tar):
        scroll = re.compile(r"var.*/scrollkeeper/?$")
        for i in tar.getnames():
            if scroll.search(i):
                self.errors.append(("scrollkeeper-dir-exists %s", i))
