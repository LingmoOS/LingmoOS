# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import re
from Namcap.ruleclass import PkgbuildRule


class package(PkgbuildRule):
    name = "makepkgfunctions"
    description = "Looks for calls to makepkg functionality"

    def analyze(self, pkginfo, tar):
        bad_calls = ["msg", "msg2", "warning", "error", "plain"]
        regex = re.compile(r"^\s+(%s) " % "|".join(bad_calls))
        hits = set()
        for i in pkginfo.pkgbuild:
            if match := regex.match(i):
                call = match.group(1)
                hits.add(call)
        for i in hits:
            self.warnings.append(("makepkg-function-used %s", i))
