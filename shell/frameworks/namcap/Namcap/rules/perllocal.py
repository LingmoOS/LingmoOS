# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "perllocal"
    description = "Verifies the absence of perllocal.pod."

    def analyze(self, pkginfo, tar):
        for i in tar.getnames():
            if i.endswith("perllocal.pod"):
                self.errors.append(("perllocal-pod-present %s", i))
