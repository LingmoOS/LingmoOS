# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.symlink


class SymlinkTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_symlink
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('i686' 'x86_64')
url="http://www.example.com/"
license=('GPL-3.0-or-later')
depends=('glibc')
source=()
options=(!purge !zipman)
backup=(etc/imaginary_file.conf)
build() {
  true
}
package() {
  mkdir -p "${pkgdir}/usr/share"
  touch "${pkgdir}/usr/share/somedata"
  ln -s nofile "${pkgdir}/usr/share/somelink"
  ln -s ../nofile "${pkgdir}/usr/share/somelink2"
  ln -s //usr/share/somedata "${pkgdir}/usr/share/validlink"
  ln -s ../share/somedata "${pkgdir}/usr/share/validlink2"
  ln -s /usr/include/math.h "${pkgdir}/usr/share/deplink"
}
"""

    def test_symlink_files(self):
        "Package with a dangling symlink"
        pkgfile = "__namcap_test_symlink-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.symlink.package)
        self.assertEqual(
            set(r.errors),
            set(
                [
                    ("dangling-symlink %s points to %s", ("usr/share/somelink", "nofile")),
                    ("dangling-symlink %s points to %s", ("usr/share/somelink2", "../nofile")),
                ]
            ),
        )
        self.assertEqual(r.warnings, [])
        self.assertEqual(
            set(r.infos),
            set(
                [
                    ("symlink-found %s points to %s", ("usr/share/somelink", "nofile")),
                    ("symlink-found %s points to %s", ("usr/share/somelink2", "../nofile")),
                    ("symlink-found %s points to %s", ("usr/share/validlink", "//usr/share/somedata")),
                    ("symlink-found %s points to %s", ("usr/share/validlink2", "../share/somedata")),
                    ("symlink-found %s points to %s", ("usr/share/deplink", "/usr/include/math.h")),
                ]
            ),
        )
