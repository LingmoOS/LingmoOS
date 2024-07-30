# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule


class ExternalHooksRule(TarballRule):
    name = "externalhooks"
    description = "Check the .INSTALL for commands covered by hooks"
    hooked = [
        "update-desktop-database",
        "update-mime-database",
        "install-info",
        "glib-compile-schemes",
        "gtk-update-icon-cache",
        "xdg-icon-resource",
        "gconfpkg",
        "gio-querymodules",
        "fc-cache",
        "mkfontscale",
        "mkfontdir",
        "systemd-sysusers",
        "systemd-tmpfiles",
        "vlc-cache-gen",
    ]

    def analyze(self, pkginfo, tar):
        if ".INSTALL" not in tar.getnames():
            return
        f = tar.extractfile(".INSTALL")
        text = f.read().decode("utf-8", "ignore")
        f.close()
        for command in self.hooked:
            if command in text:
                self.warnings.append(("external-hooks %s", (command,)))
