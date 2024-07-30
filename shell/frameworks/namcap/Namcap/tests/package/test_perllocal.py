# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.perllocal


class PerllocalTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_perllocal
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
  true
}
package() {
  mkdir -p "${pkgdir}/usr/lib/perl/5.12/site-local/libmine"
  touch "${pkgdir}/usr/lib/perl/5.12/site-local/libmine/perllocal.pod"
}
"""

    def test_perllocal_files(self):
        pkgfile = "__namcap_test_perllocal-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.perllocal.package)
        self.assertEqual(r.errors, [("perllocal-pod-present %s", "usr/lib/perl/5.12/site-local/libmine/perllocal.pod")])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
