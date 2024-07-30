# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import unittest
import tempfile
import shutil

import Namcap.package

pkgbuild = """
# Maintainer: Arch Linux <archlinux@example.com>
# Contributor: Arch Linux <archlinux@example.com>

pkgname=mypackage
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc' 'foobar')
optdepends=('libabc: provides the abc feature')
provides=('yourpackage=0.9')
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


class PkgbuildLoaderTests(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.mkdtemp()
        tmpname = os.path.join(self.tmpdir, "PKGBUILD")
        with open(tmpname, "w") as f:
            f.write(pkgbuild)
        self.pkginfo = Namcap.package.load_from_pkgbuild(tmpname)

    def tearDown(self):
        shutil.rmtree(self.tmpdir)

    def test_pkgname(self):
        self.assertEqual(self.pkginfo["name"], "mypackage")
        self.assertEqual(self.pkginfo["pkgname"], "mypackage")

    def test_version(self):
        self.assertEqual(self.pkginfo["version"], "1.0-1")

    def test_pkgdesc(self):
        self.assertEqual(self.pkginfo["desc"], "A package")

    def test_url(self):
        self.assertEqual(self.pkginfo["url"], "http://www.example.com/")

    def test_depends(self):
        self.assertEqual(self.pkginfo["depends"], ["glibc", "foobar"])
        self.assertEqual(self.pkginfo["orig_depends"], ["glibc", "foobar"])

    def test_optdeps(self):
        self.assertEqual(self.pkginfo["optdepends"], ["libabc"])
        self.assertEqual(self.pkginfo["orig_optdepends"], ["libabc: provides the abc feature"])

    def test_provides(self):
        self.assertEqual(self.pkginfo["provides"], ["yourpackage"])
        self.assertEqual(self.pkginfo["orig_provides"], ["yourpackage=0.9"])
