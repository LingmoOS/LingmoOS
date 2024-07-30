# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.permissions


class PermissionsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_permissions
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
  install -d -m 755 ${pkgdir}/usr/bin

  touch ${pkgdir}/usr/bin/program
  chmod 777 ${pkgdir}/usr/bin/program

  touch ${pkgdir}/usr/bin/secret
  chmod 700 ${pkgdir}/usr/bin/secret

  touch ${pkgdir}/usr/bin/unsafe
  chmod u+rwxs ${pkgdir}/usr/bin/unsafe

  install -d -m 744 ${pkgdir}/usr/share/broken

  install -d -m 755 ${pkgdir}/usr/lib/libstatic.a
  ln -s libstatic.a ${pkgdir}/usr/lib/libstaticlink.a
}
"""

    def test_nonrootowner(self):
        pkgfile = "__namcap_test_permissions-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.permissions.package)
        self.assertEqual(r.errors, [])
        self.assertEqual(
            set(r.warnings),
            set(
                [
                    ("file-world-writable %s", "usr/bin/program"),
                    ("file-not-world-readable %s", "usr/bin/secret"),
                    ("file-setugid %s", "usr/bin/unsafe"),
                    ("directory-not-world-executable %s", "usr/share/broken"),
                    ("incorrect-library-permissions %s", "usr/lib/libstatic.a"),
                ]
            ),
        )
        self.assertEqual(r.infos, [])
