# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import tarfile
import Namcap.rules.licensepkg
from contextlib import nullcontext as does_not_raise
from Namcap.tests.makepkg import MakepkgTest
from Namcap.rules import licensepkg
from pathlib import Path
from pytest import mark, raises
from license_expression import BaseSymbol, LicenseSymbol, LicenseWithExceptionSymbol
from tarfile import TarInfo
from typing import Any, ContextManager


@mark.parametrize(
    "input, result, expectation",
    [
        ("LicenseRef-FOO", "LicenseRef-FOO", does_not_raise()),
        ("() broken", "", raises(Exception)),
        ("MIT and MIT", "MIT AND MIT", does_not_raise()),
        ("MIT AND Apache-2.0", "MIT AND Apache-2.0", does_not_raise()),
        (
            "Apache-2.0 or (MIT with Bootloader-exception)",
            "Apache-2.0 OR MIT WITH Bootloader-exception",
            does_not_raise(),
        ),
    ],
)
def test_get_license_canonicalized(input: str, result: str, expectation: ContextManager[Any]) -> None:
    with expectation:
        assert result == licensepkg.get_license_canonicalized(input)


@mark.parametrize(
    "input, result",
    [
        ("LicenseRef-FOO+", "LicenseRef-FOO+"),
        ("MIT", "MIT"),
        ("MIT+", "MIT"),
        ("Apache-2.0+", "Apache-2.0"),
        ("GPL-3.0-only+", "GPL-3.0-only+"),
        ("GPL-3.0-or-later+", "GPL-3.0-or-later+"),
    ],
)
def test_strip_plus_from_license(input: str, result: str) -> None:
    assert LicenseSymbol(result) == licensepkg.strip_plus_from_license(LicenseSymbol(input))


@mark.parametrize(
    "input, result, expectation",
    [
        ("Foo", {LicenseSymbol("Foo")}, does_not_raise()),
        ("() broken", set(), raises(Exception)),
        ("LicenseRef-FOO", {LicenseSymbol("LicenseRef-FOO")}, does_not_raise()),
        ("MIT AND MIT", {LicenseSymbol("MIT")}, does_not_raise()),
        ("MIT AND Apache-2.0", {LicenseSymbol("MIT"), LicenseSymbol("Apache-2.0")}, does_not_raise()),
        ("MIT+ OR Apache-2.0+", {LicenseSymbol("MIT"), LicenseSymbol("Apache-2.0")}, does_not_raise()),
        (
            "Apache-2.0 OR (MIT WITH Bootloader-exception)",
            {
                LicenseSymbol("Apache-2.0"),
                LicenseWithExceptionSymbol(
                    LicenseSymbol("MIT"), LicenseSymbol("Bootloader-exception", is_exception=True)
                ),
            },
            does_not_raise(),
        ),
        (
            "Apache-2.0+ WITH Bootloader-exception",
            {
                LicenseWithExceptionSymbol(
                    LicenseSymbol("Apache-2.0"), LicenseSymbol("Bootloader-exception", is_exception=True)
                ),
            },
            does_not_raise(),
        ),
    ],
)
def test_get_license_symbols(input: str, result: set[LicenseSymbol], expectation: ContextManager[Any]) -> None:
    with expectation:
        assert result == licensepkg.get_license_symbols(input)


@mark.parametrize(
    "input, result",
    [
        ({LicenseSymbol("GPL-3.0-or-later")}, set()),
        ({LicenseSymbol("MIT")}, {LicenseSymbol("MIT")}),
        ({LicenseSymbol("LicenseRef-FOO")}, {LicenseSymbol("LicenseRef-FOO")}),
        (
            {
                LicenseWithExceptionSymbol(
                    LicenseSymbol("MIT"), LicenseSymbol("Bootloader-exception", is_exception=True)
                )
            },
            {LicenseSymbol("MIT"), LicenseSymbol("Bootloader-exception", is_exception=True)},
        ),
    ],
)
def test_get_uncommon_license_symbols(input: set[BaseSymbol], result: set[BaseSymbol]) -> None:
    assert result == licensepkg.get_uncommon_license_symbols(
        input,
        licensepkg.get_known_spdx_license_identifiers(),
        licensepkg.get_known_spdx_license_exceptions(),
        licensepkg.get_common_spdx_license_identifiers(),
        licensepkg.get_common_spdx_license_exceptions(),
    )


@mark.parametrize(
    "input, result",
    [
        ({LicenseSymbol("GPL-3.0-or-later")}, set()),
        ({LicenseSymbol("LicenseRef-FOO")}, {LicenseSymbol("LicenseRef-FOO")}),
        ({LicenseSymbol("GPL-3.0-only+")}, {LicenseSymbol("GPL-3.0-only+")}),
        (
            {
                LicenseWithExceptionSymbol(
                    LicenseSymbol("MIT"), LicenseSymbol("Bootloader-exception", is_exception=True)
                )
            },
            set(),
        ),
    ],
)
def test_get_unknown_license_symbols(input: set[BaseSymbol], result: set[BaseSymbol]) -> None:
    assert result == licensepkg.get_unknown_license_symbols(
        input, licensepkg.get_known_spdx_license_identifiers(), licensepkg.get_known_spdx_license_exceptions()
    )


@mark.parametrize(
    "create_file, create_absolute_symlink, create_relative_symlink, create_license_dir_symlink, result",
    [
        (True, False, False, False, 1),
        (True, True, False, False, 2),
        (False, True, False, False, 1),
        (False, True, True, False, 2),
        (False, False, True, False, 1),
        (False, True, False, True, 1),
        (False, False, True, True, 1),
        (True, True, True, False, 3),
        (False, False, False, False, 0),
    ],
)
def test_package_license_files(
    create_file: bool,
    create_absolute_symlink: bool,
    create_relative_symlink: bool,
    create_license_dir_symlink: bool,
    result: int,
    tmp_path: Path,
) -> None:
    with tarfile.open(tmp_path / "test.tar.gz", "w:gz") as tar:
        tar.add(tmp_path, arcname="/usr/share/foo/")

        if create_license_dir_symlink:
            if create_absolute_symlink:
                license_dir = tmp_path / "usr/share/foo/"
                license_dir.mkdir(parents=True, exist_ok=True)
                license_file = license_dir / "LICENSE.absolute"
                license_file.touch()
                tar.add(license_file, arcname="/usr/share/foo/LICENSE.absolute")

                link_dir = tmp_path / "usr/share/licenses/"
                link_dir.mkdir(parents=True, exist_ok=True)
                link_file = link_dir / "test/"
                link_file.symlink_to(Path("/usr/share/foo/"))
                tar.add(link_file, arcname="/usr/share/licenses/test/")
            if create_relative_symlink:
                license_dir = tmp_path / "usr/share/foo/"
                license_dir.mkdir(parents=True, exist_ok=True)
                license_file = license_dir / "LICENSE.relative"
                license_file.touch()
                tar.add(license_file, arcname="/usr/share/foo/LICENSE.relative")

                link_dir = tmp_path / "usr/share/licenses/"
                link_dir.mkdir(parents=True, exist_ok=True)
                link_file = link_dir / "test/"
                link_file.symlink_to(Path("../foo/"))
                tar.add(link_file, arcname="/usr/share/licenses/test/")
        else:
            tar.add(tmp_path, arcname="/usr/share/licenses/test/")
            if create_file:
                license_file = tmp_path / "LICENSE.file"
                license_file.touch()
                tar.add(license_file, arcname="/usr/share/licenses/test/LICENSE.file")
            if create_absolute_symlink:
                license_dir = tmp_path / "usr/share/foo/"
                license_dir.mkdir(parents=True, exist_ok=True)
                license_file = license_dir / "LICENSE.absolute"
                license_file.touch()
                tar.add(license_file, arcname="/usr/share/foo/LICENSE.absolute")

                link_dir = tmp_path / "usr/share/licenses/test/"
                link_dir.mkdir(parents=True, exist_ok=True)
                link_file = link_dir / "LICENSE.absolute"
                link_file.symlink_to(Path("/usr/share/foo/LICENSE.absolute"))
                tar.add(link_file, arcname="/usr/share/licenses/test/LICENSE.absolute")
            if create_relative_symlink:
                license_dir = tmp_path / "usr/share/foo/"
                license_dir.mkdir(parents=True, exist_ok=True)
                license_file = license_dir / "LICENSE.relative"
                license_file.touch()
                tar.add(license_file, arcname="/usr/share/foo/LICENSE.relative")

                link_dir = tmp_path / "usr/share/licenses/test/"
                link_dir.mkdir(parents=True, exist_ok=True)
                link_file = link_dir / "LICENSE.relative"
                link_file.symlink_to(Path("../../foo/LICENSE.relative"))
                tar.add(link_file, arcname="/usr/share/licenses/test/LICENSE.relative")

        assert result == len(licensepkg.package_license_files(tar, "test")[0])


@mark.parametrize(
    "name, linkname, result, expectation",
    [
        ("foo", "bar", "bar", does_not_raise()),
        ("foo", "/bar", "bar", does_not_raise()),
        ("foo", "../bar", None, raises(ValueError)),
    ],
)
def test_get_symlink_target(name: str, linkname: str, result: str, expectation: ContextManager[Any]) -> None:
    tarinfo = TarInfo(name)
    tarinfo.linkname = linkname

    with expectation:
        assert result == licensepkg.get_symlink_target(tarinfo)


class LicenseFileTest(MakepkgTest):
    def test_common_license_requires_no_file(self):
        pkgbuild = """
        pkgname=__namcap_test_licensepkg
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
          mkdir -p "${pkgdir}/usr/share"
          touch "${pkgdir}/usr/share/somefile"
        }
        """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(r.errors, [])
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_unknown_license_identifier_without_licenseref(self):
        pkgbuild = """
            pkgname=__namcap_test_licensepkg
            pkgver=1.0
            pkgrel=1
            pkgdesc="A package"
            arch=('i686' 'x86_64')
            url="http://www.example.com/"
            license=('DWTFYWL')
            depends=('glibc')
            source=()
            options=(!purge !zipman)
            build() {
              true
            }
            package() {
              mkdir -p "${pkgdir}/usr/share"
              touch "${pkgdir}/usr/share/somefile"
            }
            """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                ("unknown-spdx-license-identifier %s", ("DWTFYWL",)),
            ],
        )
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_license_file_for_licenseref(self):
        pkgbuild = """
            pkgname=__namcap_test_licensepkg
            pkgver=1.0
            pkgrel=1
            pkgdesc="A package"
            arch=('i686' 'x86_64')
            url="http://www.example.com/"
            license=('LicenseRef-DWTFYWL')
            depends=('glibc')
            source=()
            options=(!purge !zipman)
            build() {
              true
            }
            package() {
              mkdir -p "${pkgdir}/usr/share/licenses/${pkgname}"
              touch "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
            }
            """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [],
        )
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_missing_license_file_for_licenseref(self):
        pkgbuild = """
            pkgname=__namcap_test_licensepkg
            pkgver=1.0
            pkgrel=1
            pkgdesc="A package"
            arch=('i686' 'x86_64')
            url="http://www.example.com/"
            license=('LicenseRef-DWTFYWL')
            depends=('glibc')
            source=()
            options=(!purge !zipman)
            build() {
              true
            }
            package() {
              mkdir -p "${pkgdir}/usr/share"
              touch "${pkgdir}/usr/share/somefile"
            }
            """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                (
                    "license-file-missing %s %s %s",
                    ("LicenseRef-DWTFYWL", "__namcap_test_licensepkg", "0/1"),
                ),
            ],
        )
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_license_string_needs_formatting(self):
        pkgbuild = """
            pkgname=__namcap_test_licensepkg
            pkgver=1.0
            pkgrel=1
            pkgdesc="A package"
            arch=('i686' 'x86_64')
            url="http://www.example.com/"
            license=('GPL-3.0-or-later with Bootloader-exception')
            depends=('glibc')
            source=()
            options=(!purge !zipman)
            build() {
              true
            }
            package() {
              mkdir -p "${pkgdir}/usr/share"
              touch "${pkgdir}/usr/share/somefile"
            }
            """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                (
                    "license-statement-formatting %s %s",
                    ("GPL-3.0-or-later with Bootloader-exception", "GPL-3.0-or-later WITH Bootloader-exception"),
                ),
                ("license-file-missing %s %s %s", ("Bootloader-exception", "__namcap_test_licensepkg", "0/1")),
            ],
        )
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_invalid_license_string(self):
        pkgbuild = """
            pkgname=__namcap_test_licensepkg
            pkgver=1.0
            pkgrel=1
            pkgdesc="A package"
            arch=('i686' 'x86_64')
            url="http://www.example.com/"
            license=('() broken')
            depends=('glibc')
            source=()
            options=(!purge !zipman)
            build() {
              true
            }
            package() {
              mkdir -p "${pkgdir}/usr/share"
              touch "${pkgdir}/usr/share/somefile"
            }
            """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                (
                    "invalid-license-string %s",
                    ("() broken",),
                ),
            ],
        )
        self.assertEqual(r.warnings, [])
        self.assertEqual(r.infos, [])

    def test_symlink_to_missing_file_in_other_pkg(self):
        pkgbuild = """
        pkgname=__namcap_test_licensepkg
        pkgver=1.0
        pkgrel=1
        pkgdesc="A package"
        arch=('i686' 'x86_64')
        url="http://www.example.com/"
        license=('MIT')
        depends=('glibc')
        source=()
        options=(!purge !zipman)
        build() {
          true
        }
        package() {
          mkdir -p "${pkgdir}/usr/share/licenses/__namcap_test_licensepkg/"
          ln -sf /usr/share/licenses/other_pkg/LICENSE "${pkgdir}/usr/share/licenses/__namcap_test_licensepkg/"
        }
        """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                (
                    "license-file-missing-in-other-pkg %s",
                    ("usr/share/licenses/other_pkg/LICENSE",),
                ),
            ],
        )
        self.assertEqual(r.warnings, [("license-file-in-external-pkg %s", ("usr/share/licenses/other_pkg/LICENSE",))])
        self.assertEqual(r.infos, [])

    def test_license_dir_is_symlink_and_target_pkg_not_in_depends(self):
        pkgbuild = """
        pkgname=__namcap_test_licensepkg
        pkgver=1.0
        pkgrel=1
        pkgdesc="A package"
        arch=('i686' 'x86_64')
        url="http://www.example.com/"
        license=('MIT')
        depends=('glibc')
        source=()
        options=(!purge !zipman)
        build() {
          true
        }
        package() {
          mkdir -p "${pkgdir}/usr/share/licenses/"
          ln -sf /usr/share/licenses/other_pkg/ "${pkgdir}/usr/share/licenses/__namcap_test_licensepkg"
        }
        """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                ("license-dir-target-pkg-not-in-depends %s %s %s", ("other_pkg", "MIT", "0/1")),
            ],
        )
        self.assertEqual(r.warnings, [("license-dir-in-external-pkg %s", ("usr/share/licenses/other_pkg",))])
        self.assertEqual(r.infos, [])

    def test_license_dir_is_symlink_and_dependencies_do_not_provide_files(self):
        pkgbuild = """
        pkgname=__namcap_test_licensepkg
        pkgver=1.0
        pkgrel=1
        pkgdesc="A package"
        arch=('i686' 'x86_64')
        url="http://www.example.com/"
        license=('MIT')
        depends=('licenses')
        source=()
        options=(!purge !zipman)
        build() {
          true
        }
        package() {
          mkdir -p "${pkgdir}/usr/share/licenses/"
          ln -sf /usr/share/foo/ "${pkgdir}/usr/share/licenses/__namcap_test_licensepkg"
        }
        """
        pkgfile = "__namcap_test_licensepkg-1.0-1-%(arch)s.pkg.tar" % {"arch": self.arch}
        with open(os.path.join(self.tmpdir, "PKGBUILD"), "w") as f:
            f.write(pkgbuild)
        self.run_makepkg()
        pkg, r = self.run_rule_on_tarball(os.path.join(self.tmpdir, pkgfile), Namcap.rules.licensepkg.package)
        self.assertEqual(
            r.errors,
            [
                (
                    "license-dir-is-symlink-and-license-files-missing %s %s %s",
                    ("MIT", "usr/share/foo", "0/1"),
                ),
            ],
        )
        self.assertEqual(r.warnings, [("license-dir-in-external-pkg %s", ("usr/share/foo",))])
        self.assertEqual(r.infos, [])
