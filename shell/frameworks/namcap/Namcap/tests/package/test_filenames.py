# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.filenames


class FilenamesTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_nonascii
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
  mkdir -p "${pkgdir}/usr/bin/Arch Linux"
  mkdir -p ${pkgdir}/usr/bin/Arch·Linux
  mkdir -p ${pkgdir}/usr/bin/$'Arch\\xb7Linux'
}
"""

    def test_nonascii(self):
        pkgfile = "__namcap_test_nonascii-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w", encoding="utf-8") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.filenames.package)
        self.assertEqual(r.errors, [])
        tag, value1 = r.warnings[0]
        tag, value2 = r.warnings[1]
        self.assertEqual(tag, "invalid-filename")
        self.assertEqual(
            set((value1.encode(errors="surrogateescape"), value2.encode(errors="surrogateescape"))),
            set((b"usr/bin/Arch\xc2\xb7Linux", b"usr/bin/Arch\xb7Linux")),
        )
        self.assertEqual(r.infos, [])
