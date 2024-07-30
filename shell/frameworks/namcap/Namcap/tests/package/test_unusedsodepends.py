# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.unusedsodepends


class UnusedSodependsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_unusedsodepends
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
  cd "${srcdir}"
  echo "int main() { return 0; }" > main.c
  /usr/bin/gcc -o main main.c -lm
}
package() {
  install -D -m 755 "$srcdir/main" "$pkgdir/usr/bin/evilprogram"
}
"""

    def test_unusedsodepends_files(self):
        "Package with binaries linking to unused shared libraries"
        pkgfile = "__namcap_test_unusedsodepends-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.unusedsodepends.package)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("unused-sodepend %s %s", ("/usr/lib/libm.so.6", "usr/bin/evilprogram"))])
        self.assertEqual(r.infos, [])
