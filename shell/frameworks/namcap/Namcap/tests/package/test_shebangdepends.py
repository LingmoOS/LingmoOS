# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.shebangdepends


class ShebangDependsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_shebangdepends
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('any')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=()
source=()
options=(!purge !zipman)
build() {
  cd "${srcdir}"
  echo -e "#! /usr/bin/env python\nprint('a script')" > python_sample
  echo -e "#!/bin\\xffary/da\\x00ta\ncrash?" > binary_sample
}
package() {
  install -Dm755 "$srcdir/python_sample" "$pkgdir/usr/bin/python_sample"
  install -Dm755 "$srcdir/binary_sample" "$pkgdir/usr/share/binary_sample"
}
"""

    def test_shebangdepends(self):
        "Package with missing python dependency"
        pkgfile = "__namcap_test_shebangdepends-1.0-1-any.pkg.tar"
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.shebangdepends.ShebangDependsRule
        )
        e, w, i = Namcap.depends.analyze_depends(pkg)
        self.assertEqual(
            e,
            [
                (
                    "dependency-detected-not-included %s (%s)",
                    ("python", "programs ['python'] needed in scripts ['usr/bin/python_sample']"),
                )
            ],
        )
        self.assertEqual(w, [])
