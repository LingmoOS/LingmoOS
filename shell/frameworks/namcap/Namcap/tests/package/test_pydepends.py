# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.pydepends


class PyDependsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_pydepends
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('any')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('python-six')
source=()
build() {
  cd "${srcdir}"
  echo "import six, pyalpm" > main.py
  echo "(^y^)" > invalid.py
}
package() {
  install -D -m 755 "$srcdir/main.py" "$pkgdir/usr/bin/main.py"
  install -D -m 755 "$srcdir/invalid.py" "$pkgdir/usr/bin/invalid.py"
}
"""

    def test_pydepends(self):
        "Package with missing pacman dependency"
        pkgfile = "__namcap_test_pydepends-1.0-1-any.pkg.tar"
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.pydepends.PythonDependencyRule
        )
        self.assertEqual(
            pkg.detected_deps["pyalpm"], [("python-modules-needed %s %s", (str(["pyalpm"]), str(["usr/bin/main.py"])))]
        )
        e, w, i = Namcap.depends.analyze_depends(pkg)
        self.assertEqual(
            e,
            [
                (
                    "dependency-detected-not-included %s (%s)",
                    ("pyalpm", "python modules ['pyalpm'] needed in files ['usr/bin/main.py']"),
                )
            ],
        )
        self.assertEqual(w, [])
