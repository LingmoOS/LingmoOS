# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re
from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "libtool"
    description = "Checks for libtool (*.la) files."

    def analyze(self, pkginfo, tar):
        for i in tar.getnames():
            if re.search(r"\.la$", i) is not None:
                self.warnings.append(("libtool-file-present %s", i))
