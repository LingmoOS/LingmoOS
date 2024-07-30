# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "sfurl"
    description = "Checks for proper sourceforge URLs"

    def analyze(self, pkginfo, tar):
        if "source" in pkginfo:
            for source in pkginfo["source"]:
                if re.match(r"(http://|ftp://)\w+.dl.(sourceforge|sf).net", source):
                    self.warnings.append(("specific-sourceforge-mirror", ()))
                if re.match(r"(http://|ftp://)dl.(sourceforge|sf).net", source):
                    self.warnings.append(("using-dl-sourceforge", ()))
