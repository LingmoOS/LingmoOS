# Copyright (C) 2024 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.dbus1location


class Dbus1locationTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_dbus1location
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
  mkdir -p "${pkgdir}/etc/dbus-1/system.d"
  touch "${pkgdir}/etc/dbus-1/system.d/foo.conf"
}
"""

    def test_dbus1location(self):
        pkgfile = "__namcap_test_dbus1location-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.dbus1location.dbus1locationRule
        )
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("dbus-1-location %s", "etc/dbus-1/system.d/foo.conf")])
        self.assertEqual(r.infos, [])
