# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.hardlinks


class HardlinksTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_hardlinks
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
  mkdir -p "${pkgdir}/usr/bin"
  mkdir -p "${pkgdir}/usr/sbin"
  touch "${pkgdir}/usr/bin/prog1"
  ln "${pkgdir}/usr/bin/prog1" "${pkgdir}/usr/sbin/prog2"
}
"""

    def test_crossdir_hardlink(self):
        "Package with hard links crossing directories"
        pkgfile = "__namcap_test_hardlinks-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.hardlinks.package)
        errors = [(s, set(tup)) for (s, tup) in r.errors]
        self.assertEqual(errors, [("cross-dir-hardlink %s %s", set(("usr/bin/prog1", "usr/sbin/prog2")))])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
