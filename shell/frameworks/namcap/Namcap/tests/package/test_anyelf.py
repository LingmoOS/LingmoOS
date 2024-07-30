# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.anyelf


class AnyElfTest(MakepkgTest):
    pkgbuild_any = """
pkgname=__namcap_test_anyelf
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
options=(emptydirs)
source=()
build() {
  true
}
package() {
  mkdir -p ${pkgdir}/usr/share/directory
  touch ${pkgdir}/usr/share/directory_file
}
"""
    pkgbuild_elf = """
pkgname=__namcap_test_anyelf
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('any')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
options=(emptydirs)
source=()
build() {
  true
}
package() {
  install -m755 -D /bin/ls ${pkgdir}/usr/bin/ls
}
"""

    pkgbuild_static = """
pkgname=__namcap_test_anyelf
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('any')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
options=(emptydirs)
source=()
build() {
  echo "void function() { return; }" > main.c
  gcc -c main.c
  ar cru library.a main.o
}
package() {
  install -m644 -D ${srcdir}/library.a ${pkgdir}/usr/lib/library.a
}
"""

    def test_not_any_no_elf(self):
        pkgfile = "__namcap_test_anyelf-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild_any)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.anyelf.package)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("no-elffiles-not-any-package", ())])
        self.assertEqual(r.infos, [])

    def test_any_but_elf(self):
        pkgfile = "__namcap_test_anyelf-1.0-1-any.pkg.tar"
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild_elf)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.anyelf.package)
        self.assertEqual(r.errors, [("elffile-in-any-package %s", "usr/bin/ls")])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_static(self):
        pkgfile = "__namcap_test_anyelf-1.0-1-any.pkg.tar"
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild_static)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.anyelf.package)
        self.assertEqual(r.errors, [("elffile-in-any-package %s", "usr/lib/library.a")])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
