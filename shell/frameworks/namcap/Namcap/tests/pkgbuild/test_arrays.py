# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.tests.pkgbuild_test import PkgbuildTest
import Namcap.rules.arrays


class NamcapArraysTest(PkgbuildTest):
    pkgbuild1 = """
# Maintainer: Arch Linux <archlinux@example.com>
# Contributor: Arch Linux <archlinux@example.com>

pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license='GPL-3.0-or-later'
depends='glibc'
options=('!libtool')
source=(ftp://ftp.example.com/pub/mypackage-0.1.tar.gz)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make
}

package() {
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install
}
"""
    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = Namcap.rules.arrays.package

    def test_example1(self):
        "Variables in PKGBUILD are not an array"
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [])
        self.assertEqual(set(r.warnings), set(("variable-not-array %s", (i,)) for i in ["depends", "license"]))
        self.assertEqual(r.infos, [])
