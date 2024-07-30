# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.rpath


class RpathTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_rpath
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
  /usr/bin/gcc -o main.o -c main.c
  # core/binutils defaults to DT_RUNPATH since 2.39-4. Force old DT_RPATH here for testing.
  /usr/bin/ld -o main --disable-new-dtags --rpath /home/evil/lib main.o -lc
}
package() {
  install -D -m 755 "$srcdir/main" "$pkgdir/usr/bin/evilprogram"
}
"""

    def test_rpath_files(self):
        "Package with fancy RPATHs"
        pkgfile = "__namcap_test_rpath-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.rpath.package)
        self.assertEqual(r.errors, [("insecure-rpath %s %s", ("/home/evil/lib", "usr/bin/evilprogram"))])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
