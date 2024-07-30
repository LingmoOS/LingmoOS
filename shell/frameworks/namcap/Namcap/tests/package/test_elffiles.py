# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
from Namcap.tests.makepkg import MakepkgTest
import Namcap.rules.elffiles


class FHSELFTest(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_elffiles
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
  install -D -m 644 /bin/ls "${pkgdir}/usr/share/something/binary"
}
"""

    def test_fhs_elffiles(self):
        pkgfile = "__namcap_test_elffiles-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.elffiles.ELFPaths)
        self.assertEqual(r.errors, [("elffile-not-in-allowed-dirs %s", ("usr/share/something/binary",))])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])


class TestExecStack(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_execstack
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
  /usr/bin/gcc -o main -Wa,-execstack main.c
}
package() {
  install -D -m 644 "${srcdir}/main" "${pkgdir}/usr/bin/withexecstack"
}
"""

    def test_execstack(self):
        pkgfile = "__namcap_test_execstack-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.elffiles.ELFExecStackRule)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("elffile-with-execstack %s", ("usr/bin/withexecstack",))])
        self.assertEqual(r.infos, [])


class TestNoPieStack(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_nopie
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
  /usr/bin/gcc -o main main.c -no-pie
}
package() {
  install -D -m 644 "${srcdir}/main" "${pkgdir}/usr/bin/nopie"
}
"""

    def test_nopie(self):
        pkgfile = "__namcap_test_nopie-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.elffiles.NoPIERule)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [("elffile-nopie %s", ("usr/bin/nopie",))])
        self.assertEqual(r.infos, [])


class TestNoShadowStack(MakepkgTest):
    pkgbuild = """
pkgname=__namcap_test_noshstk
pkgver=1.0
pkgrel=1
pkgdesc="A package"
arch=('x86_64')
url="http://www.example.com/"
license=('GPL')
depends=('glibc')
source=()
options=(!purge !zipman)
build() {
  cd "${srcdir}"
  echo "int main() { return 0; }" > main.c
  /usr/bin/gcc -o fcf-protection-branch main.c -fcf-protection=branch
  /usr/bin/gcc -o fcf-protection-full main.c -fcf-protection=full
  /usr/bin/gcc -o fcf-protection-none main.c -fcf-protection=none
  /usr/bin/gcc -o fcf-protection-none-m32 main.c -fcf-protection=none -m32 -nostdlib
  /usr/bin/gcc -o fcf-protection-return main.c -fcf-protection=return
  /usr/bin/objcopy --remove-section=.note.gnu.property fcf-protection-full no-.note.gnu.property
}
package() {
  install -D -m 644 "${srcdir}/fcf-protection-branch" "${pkgdir}/usr/bin/fcf-protection-branch"
  install -D -m 644 "${srcdir}/fcf-protection-full" "${pkgdir}/usr/bin/fcf-protection-full"
  install -D -m 644 "${srcdir}/fcf-protection-none" "${pkgdir}/usr/bin/fcf-protection-none"
  install -D -m 644 "${srcdir}/fcf-protection-none-m32" "${pkgdir}/usr/bin/fcf-protection-none-m32"
  install -D -m 644 "${srcdir}/fcf-protection-return" "${pkgdir}/usr/bin/fcf-protection-return"
  install -D -m 644 "${srcdir}/no-.note.gnu.property" "${pkgdir}/usr/bin/no-.note.gnu.property"
}
"""

    def test_noshstk(self):
        pkgfile = "__namcap_test_noshstk-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(self.pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.elffiles.ELFSHSTKRule)
        self.assertEqual(r.errors, [])
        self.assertEqual(
            r.warnings,
            [
                ("elffile-noshstk %s", ("usr/bin/fcf-protection-branch",)),
                ("elffile-noshstk %s", ("usr/bin/fcf-protection-none",)),
                ("elffile-noshstk %s", ("usr/bin/no-.note.gnu.property",)),
            ],
        )
        self.assertEqual(r.infos, [])
