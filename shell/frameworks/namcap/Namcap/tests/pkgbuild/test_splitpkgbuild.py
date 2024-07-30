# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.tests.pkgbuild_test import PkgbuildTest
import Namcap.rules.splitpkgbuild as module


class NamcapSplitPkgbuildTest(PkgbuildTest):
    pkgbuild1 = """
# Maintainer: Arch Linux <archlinux@example.com>
# Contributor: Arch Linux <archlinux@example.com>

pkgbase=mypackage
pkgname=('prog1' 'prog2')
pkgver=1.0
pkgrel=1
pkgdesc="A package"
url="http://mypackage.sf.net/"
arch=('i686' 'x86_64')
depends=('glibc')
license=('GPL-3.0-or-later')
options=('!libtool')
source=(ftp://heanet.dl.sourceforge.net/pub/m/mypackage/mypackage-0.1.tar.gz)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make
}

package_prog1() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install1
}
"""

    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = module.PackageFunctionsRule

    def test_example1(self):
        "Example 1 : missing package_* function"
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [("missing-pkgfunction %s", ("prog2",))])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])


class NamcapSplitMakedepsTest(PkgbuildTest):
    pkgbuild1 = """
# Maintainer: Arch Linux <archlinux@example.com>
# Contributor: Arch Linux <archlinux@example.com>

pkgbase=mypackage
pkgname=('prog1' 'prog2')
pkgver=1.0
pkgrel=1
pkgdesc="A package"
url="http://mypackage.sf.net/"
arch=('i686' 'x86_64')
makedepends=('gtk2')
license=('GPL-3.0-or-later')
options=('!libtool')
source=(ftp://heanet.dl.sourceforge.net/pub/m/mypackage/mypackage-0.1.tar.gz)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make
}

package_prog1() {
  depends=("gtk2")
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install1
}

package_prog2() {
  depends=("lib1" "lib2")
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install1
}
"""

    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = module.SplitPkgMakedepsRule

    def test_example1(self):
        "Example 1: missing makedepend"
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [("missing-makedeps %s", (str(["lib1", "lib2"]),))])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
