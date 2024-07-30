# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class package(TarballRule):
    name = "missingbackups"
    description = "Backup files listed in package should exist"

    def analyze(self, pkginfo, tar):
        if "backup" not in pkginfo or len(pkginfo["backup"]) == 0:
            return
        known_backups = set(pkginfo["backup"])
        found_files = set(tar.getnames())
        missing_backups = known_backups - found_files
        for backup in missing_backups:
            self.errors.append(("missing-backup-file %s", backup))
