# Copyright (C) 2024 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.sphinxbuildcachefiles


class SphinxBuildCacheFilesTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_sphinxbuildcachefiles
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
source=()
build() {
  true
}
package() {
  touch ${pkgdir}/environment.pickle
}
"""

    def test_cache_files(self):
        pkgfile = "__namcap_test_sphinxbuildcachefiles-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.sphinxbuildcachefiles.sphinxbuildcachefilesRule
        )
        self.assertEqual(r.errors, [("sphinx-build-cache-files", ())])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
