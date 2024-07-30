# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.tests.pkgbuild_test import PkgbuildTest
import Namcap.rules.makepkgfunctions as module


class NamcapMakepkgfunctionsTest(PkgbuildTest):
    pkgbuild1 = """
# Maintainer: Arch Linux <archlinux at example.com>
# Contributor: Arch Linux <archlinux at example.com>

pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
options=('!libtool')
source=(ftp://ftp.example.com/pub/mypackage-0.1.tar.gz)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  msg "some text"
  msg2 "some text"
  warning "some text"
  error "some text"
  plain "some text"
}

package() {
  true
}
"""
    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = module.package

    def test_example1(self):
        bad_calls = ["msg", "msg2", "warning", "error", "plain"]
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [])
        self.assertEqual(set(r.warnings), set(("makepkg-function-used %s", i) for i in bad_calls))
        self.assertEqual(r.infos, [])
