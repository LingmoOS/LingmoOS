# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from typing import Callable, NoReturn
import unittest
import tempfile
import shutil

import Namcap.package
import Namcap.ruleclass


class PkgbuildTest(unittest.TestCase):
    """
    This class is the template for unit tests concerning PKGBUILD rules.
    The setUp()	method should set self.rule to a class derived from
    PkgbuildRule.

    The usual way to do that is by defining

    def preSetUp(self):
            self.rule = myRuleClass
            # Add standard valid PKGBUILDs to tests
            self.test_valid = PkgbuildTest.valid_tests
    """

    rule: Callable[..., Namcap.ruleclass.AbstractRule] | NoReturn

    def preSetUp(self):
        pass

    def setUp(self):
        self.preSetUp()
        self.tmpdir = tempfile.mkdtemp()
        self.tmpname = os.path.join(self.tmpdir, "PKGBUILD")

    def run_on_pkg(self, p):
        with open(self.tmpname, "w", encoding="utf-8") as f:
            f.write(p)
        pkginfo = Namcap.package.load_from_pkgbuild(self.tmpname)
        r = self.rule()
        if isinstance(r, Namcap.ruleclass.PkgInfoRule):
            if pkginfo.is_split:
                for p in pkginfo.subpackages:
                    r.analyze(p, self.tmpname)
            else:
                r.analyze(pkginfo, self.tmpname)
        if isinstance(r, Namcap.ruleclass.PkgbuildRule):
            r.analyze(pkginfo, self.tmpname)
        return r

    def valid_tests(self):
        "Valid PKGBUILDS"
        for p in valid_pkgbuilds:
            r = self.run_on_pkg(p)
            self.assertEqual(r.errors, [])
            self.assertEqual(r.warnings, [])
            self.assertEqual(r.errors, [])

    def tearDown(self):
        shutil.rmtree(self.tmpdir)


_basic = """
# Maintainer: Arch Linux <archlinux@example.com>
# Contributor: Arch Linux <archlinux@example.com>

pkgname=mypackage
pkgver=1.0
pkgrel=1
epoch=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc' 'pacman')
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
  make DESTDIR="${pkgdir}" install
}
"""

_split = """
# Maintainer: Arch Linux <archlinux@example.com>
# Contributor: Arch Linux <archlinux@example.com>

pkgbase=mysplitpackage
pkgname=('mypackage1' 'mypackage2' 'mypackage3')
pkgver=1.0
pkgrel=1
epoch=2
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
makedepends=('make' 'python')
checkdepends=('perl')
options=('!libtool')
source=(ftp://ftp.example.com/pub/mypackage-0.1.tar.gz)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  cd "${srcdir}"/${pkgbase}-${pkgver}
  ./configure --prefix=/usr
  make
}

check() {
  cd "${srcdir}"/${pkgbase}-${pkgver}
  make check
}

package_mypackage1() {
  pkgdesc="Package 1"
  depends=("python")
  cd "${srcdir}"/${pkgbase}-${pkgver}
  make DESTDIR="${pkgdir}" install1
}

package_mypackage2() {
  pkgdesc="Package 2"
  depends=("zlib" "mypackage1")
  cd "${srcdir}"/${pkgbase}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install2
}

package_mypackage3() {
  pkgdesc="Package 3"
  depends=("zlib")
  optdepends=("mypackage1: for foobar functionality")
  install=somescript.install
  cd "${srcdir}"/${pkgname}-${pkgver}
  ./configure --prefix=/usr
  make DESTDIR="${pkgdir}" install3
}
"""

valid_pkgbuilds = [_basic, _split]
