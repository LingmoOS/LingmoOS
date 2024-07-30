# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.lotsofdocs


class LotsOfDocsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_lotsofdocs
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
source=()
options=(!purge !zipman)
build() {
  true
}
package() {
  mkdir -p "${pkgdir}/usr/share/doc"
  dd if=/dev/zero of="${pkgdir}/usr/share/doc/somefile" bs=1k count=500
}
"""

    def test_lotsofdocs(self):
        "Empty package with a 500kB file in /usr/share/doc"
        pkgfile = "__namcap_test_lotsofdocs-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.lotsofdocs.package)
        self.assertEqual(r.errors, [])
        tag, value = r.warnings[0]
        self.assertEqual(tag, "lots-of-docs %f")
        self.assertGreater(value, (50,))
        self.assertEqual(r.infos, [])
