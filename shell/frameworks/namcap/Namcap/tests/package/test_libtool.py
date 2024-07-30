# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.libtool


class LibtoolTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_libtool
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
  mkdir -p "${pkgdir}/usr/lib"
  touch "${pkgdir}/usr/lib/libsomething.la"
}
"""

    def test_libtool_files(self):
        pkgfile = "__namcap_test_libtool-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.libtool.package)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("libtool-file-present %s", "usr/lib/libsomething.la")])
        self.assertEqual(r.infos, [])
