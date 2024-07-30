# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.sodepends
from elftools.elf.dynamic import DynamicSection
from elftools.elf.elffile import ELFFile


def get_soname(filename):
    with open(filename, "rb") as f:
        alpm = ELFFile(f)
        for section in alpm.iter_sections():
            if not isinstance(section, DynamicSection):
                continue
            for tag in section.iter_tags("DT_SONAME"):
                return tag.soname


class SoDependsTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_sodepends
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
  cd "${srcdir}"
  echo "int main() { return 0; }" > main.c
  gcc -o main main.c -Wl,--no-as-needed -lalpm
}
package() {
  install -D -m 755 "$srcdir/main" "$pkgdir/usr/bin/main"
}
"""

    def test_sodepends(self):
        "Package with missing pacman dependency"
        pkgfile = "__namcap_test_sodepends-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        alpm_filename = os.path.join("usr/lib", get_soname("/usr/lib/libalpm.so"))
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.sodepends.SharedLibsRule)
        self.assertEqual(
            pkg.detected_deps["pacman"], [("libraries-needed %s %s", (str([alpm_filename]), str(["usr/bin/main"])))]
        )
        e, w, i = Namcap.depends.analyze_depends(pkg)
        self.assertEqual(
            e,
            [
                (
                    "dependency-detected-not-included %s (%s)",
                    ("pacman", "libraries ['%s'] needed in files ['usr/bin/main']" % alpm_filename),
                )
            ],
        )
        self.assertEqual(w, [])
