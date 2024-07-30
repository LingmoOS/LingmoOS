# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "gnomemime"
    description = "Checks for generated GNOME mime files"

    def analyze(self, pkginfo, tar):
        mime_files = [
            "usr/share/applications/mimeinfo.cache",
            "usr/share/mime/XMLnamespaces",
            "usr/share/mime/aliases",
            "usr/share/mime/globs",
            "usr/share/mime/magic",
            "usr/share/mime/subclasses",
        ]

        for i in tar.getnames():
            if i in mime_files:
                self.errors.append(("gnome-mime-file %s", i))
