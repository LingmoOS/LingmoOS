# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "fileownership"
    description = "Checks file ownership."

    def analyze(self, pkginfo, tar):
        for i in tar.getmembers():
            if i.uname != "root" or i.gname != "root":
                uname = ""
                gname = ""
                if i.uname == "":
                    uname = str(i.uid)
                else:
                    uname = i.uname
                if i.gname == "":
                    gname = str(i.gid)
                else:
                    gname = i.gname
                self.errors.append(("incorrect-owner %s (%s:%s)", (i.name, uname, gname)))
