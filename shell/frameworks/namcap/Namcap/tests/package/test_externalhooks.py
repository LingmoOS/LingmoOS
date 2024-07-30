# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.externalhooks


class ExternalHooksTest(MakepkgTest):
    pkgbuild_generic = """
pkgname=__namcap_test_externalhooks
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('any')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
options=(emptydirs)
install=sample.install
source=()
build() {
  true
}
package() {
  true
}

"""
    install_bad = """
post_install() {
  update-desktop-database -q
  gtk-update-icon-cache -q -t -f usr/share/icons/hicolor
}
"""
    install_good = """
post_install() {
  true
}
"""

    def test_bad_install(self):
        pkgfile = "__namcap_test_externalhooks-1.0-1-any.pkg.tar"
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild_generic)
        with open(os.path.join(self.tmpdir, "sample.install"), "w") as f:
            f.write(self.install_bad)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.externalhooks.ExternalHooksRule
        )
        self.assertEqual(r.errors, [])
        self.assertEqual(
            r.warnings,
            [("external-hooks %s", ("update-desktop-database",)), ("external-hooks %s", ("gtk-update-icon-cache",))],
        )
        self.assertEqual(r.infos, [])

    def test_good_install(self):
        pkgfile = "__namcap_test_externalhooks-1.0-1-any.pkg.tar"
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild_generic)
        with open(os.path.join(self.tmpdir, "sample.install"), "w") as f:
            f.write(self.install_good)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(
            os.path.join(self.tmpdir, pkgfile), Namcap.rules.externalhooks.ExternalHooksRule
        )
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])
