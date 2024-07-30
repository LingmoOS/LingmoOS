# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Checks for invalid filenames."""

import string
from Namcap.ruleclass import TarballRule

VALID_CHARS = string.ascii_letters + string.digits + string.punctuation + " "


class package(TarballRule):
    name = "filenames"
    description = "Checks for invalid filenames."

    def analyze(self, pkginfo, tar):
        for i in tar.getnames():
            if not all(c in VALID_CHARS for c in i):
                self.warnings.append(("invalid-filename", i))
