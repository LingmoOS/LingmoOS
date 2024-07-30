# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.tests.pkgbuild_test import PkgbuildTest
import Namcap.rules.makedepends as module


class NamcapRedundantMakedependsTest(PkgbuildTest):
    pkgbuild1 = """
# Maintainer: Arch Linux <archlinux at example.com>
# Contributor: Arch Linux <archlinux at example.com>

pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
url="http://www.example.com/"
arch=('i686' 'x86_64')
depends=('lib1' 'lib2' 'lib3')
makedepends=('lib1' 'lib2' 'lib4')
license=('GPL-3.0-or-later')
options=('!libtool')
source=(ftp://ftp.example.com/pub/mypackage-0.1.tar.gz)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  true
}

package() {
  true
}
"""
    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = module.RedundantMakedepends

    def test_example1(self):
        # Example 1
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [])
        self.assertEqual(set(r.warnings), set(("redundant-makedep %s", (i,)) for i in ["lib1", "lib2"]))
        self.assertEqual(r.infos, [])


class NamcapVCSMakedependsTest(PkgbuildTest):
    pkgbuild1 = """
# Maintainer: Arch Linux <archlinux at example.com>
# Contributor: Arch Linux <archlinux at example.com>

pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
url="http://www.example.com/"
arch=('i686' 'x86_64')
depends=()
makedepends=()
license=('GPL-3.0-or-later')
options=('!libtool')
source=(name::bzr+https://example.com/pub/mypackage
        name::git://example.com/pub/mypackage
        hg+https://example.com/pub/mypackage
        svn://example.com/pub/mypackage)
md5sums=('abcdefabcdef12345678901234567890')

build() {
  true
}

package() {
  true
}
"""
    test_valid = PkgbuildTest.valid_tests

    def preSetUp(self):
        self.rule = module.VCSMakedepends

    def test_example1(self):
        # Example 1
        makedeps = ["breezy", "git", "mercurial", "subversion"]
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [])
        self.assertEqual(set(r.warnings), set(("missing-vcs-makedeps %s", (i,)) for i in makedeps))
        self.assertEqual(r.infos, [])

    def test_example2(self):
        # Example 2
        r = self.run_on_pkg(self.pkgbuild1 + "source=(gitsomething)")
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
