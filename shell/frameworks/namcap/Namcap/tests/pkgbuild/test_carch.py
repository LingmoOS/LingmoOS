# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.tests.pkgbuild_test import PkgbuildTest
import Namcap.rules.carch as module


class NamcapSpecialArchTest(PkgbuildTest):
    pkgbuild1 = """
pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686')

build() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  [[ $CARCH == x86_64 ]] && CFLAGS+="-m64"
  [ '$CARCH' = 'i686' ] && CFLAGS+="-m32"
  ./configure --prefix=/usr
  make
}
"""

    pkgbuild2 = """
pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686')

build() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  [[ $CARCH == x86_64 ]] && CFLAGS+="-m64"
  [ '$CARCH' = 'i686' ] && CFLAGS+="-m32"
  ./configure --prefix=/usr
  make
}

package() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install
  cp foobar /usr/lib/i686/pkg/
}
"""

    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = module.package

    def test_example1(self):
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_example2(self):
        r = self.run_on_pkg(self.pkgbuild2)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("specific-host-type-used %s", ("i686",))])
        self.assertEqual(r.infos, [])
