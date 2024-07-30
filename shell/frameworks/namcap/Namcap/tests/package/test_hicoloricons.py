# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.depends
import Namcap.rules.pathdepends


class HiColorIconsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_hicoloricons
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=()
source=()
options=(!purge !zipman)
build() {
  true
}
package() {
  mkdir -p "${pkgdir}/usr/share/icons/hicolor/64x64"
}
"""

    def test_hicoloricons_file_exists(self):
        pkgfile = "__namcap_test_hicoloricons-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.pathdepends.PathDependsRule)
        e, w, i = Namcap.depends.analyze_depends(pkg)
        self.assertEqual(pkg.detected_deps, {"hicolor-icon-theme": [("hicolor-icon-theme-needed-for-hicolor-dir", ())]})
        self.assertEqual(
            set(r.errors + e),
            set(
                [
                    (
                        "dependency-detected-not-included %s (%s)",
                        ("hicolor-icon-theme", "needed for hicolor theme hierarchy"),
                    ),
                ]
            ),
        )
        self.assertEqual(r.warnings + w, [])
        self.assertEqual(r.infos + i, [("depends-by-namcap-sight depends=(%s)", ("hicolor-icon-theme",))])

    valid_pkgbuild = """
pkgname=__namcap_test_hicoloricons
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('hicolor-icon-theme')
source=()
options=(!purge !zipman)
build() {
  true
}
package() {
  mkdir -p "${pkgdir}/usr/share/icons/hicolor/64x64"
}
"""

    def test_hicoloricons_normal(self):
        pkgfile = "__namcap_test_hicoloricons-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.valid_pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.pathdepends.PathDependsRule)
        e, w, i = Namcap.depends.analyze_depends(pkg)
        self.assertEqual(pkg.detected_deps, {"hicolor-icon-theme": [("hicolor-icon-theme-needed-for-hicolor-dir", ())]})
        self.assertEqual(r.errors + e, [])
        self.assertEqual(r.warnings + w, [])
        self.assertEqual(
            r.infos + i,
            [
                ("dependency-detected-satisfied %s (%s)", ("hicolor-icon-theme", "needed for hicolor theme hierarchy")),
                ("depends-by-namcap-sight depends=(%s)", ("hicolor-icon-theme",)),
            ],
        )
