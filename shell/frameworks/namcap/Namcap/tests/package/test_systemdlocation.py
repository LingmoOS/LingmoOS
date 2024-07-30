# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.systemdlocation


class SystemdlocationTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_systemdlocation
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
source=()
build() {
  true
}
package() {
  mkdir -p "${pkgdir}/etc/systemd/system"
  touch "${pkgdir}/etc/systemd/system/systemdsomething"
}
"""

    def test_systemdlocation_pkgname(self):
        pkgfile = "systemd-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild + "pkgname=systemd")
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.systemdlocation.systemdlocationRule
        )
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_systemdlocation_provides(self):
        pkgfile = "__namcap_test_systemdlocation-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild + "provides=(systemd)")
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.systemdlocation.systemdlocationRule
        )
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_systemdlocation(self):
        pkgfile = "__namcap_test_systemdlocation-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.systemdlocation.systemdlocationRule
        )
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("systemd-location %s", "etc/systemd/system/systemdsomething")])
        self.assertEqual(r.infos, [])
