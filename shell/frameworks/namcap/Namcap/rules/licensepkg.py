# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from license_expression import BaseSymbol, LicenseSymbol, LicenseWithExceptionSymbol, get_spdx_licensing
from Namcap.ruleclass import TarballRule
from Namcap.package import load_from_db, PacmanPackage
from Namcap.util import is_debug
from pathlib import Path
from tarfile import TarFile, TarInfo


def get_license_canonicalized(license: str) -> str:
    """Get the canonicalized form of a license string

    This function may raise an Exception if it's not possible to derive any meaning from the input string
    """
    licensing = get_spdx_licensing()
    return str(licensing.parse(license, strict=True))


def strip_plus_from_license(license: BaseSymbol) -> BaseSymbol:
    """Remove a "+" suffix from the license symbol if it contains one

    The license_expression module does not support the SPDX "+" operator, but parses it as part of the license
    symbol instead. This function strips the operator from the symbol if it is found.

    License identifiers ending with "-only" or "-or-later" should not be used with the plus operator. If such
    license identifier is detected, the plus operator is left intact and this will be later reported as an
    unknown license symbol.
    """
    license_str = str(license)
    if license_str[-1] != "+" or license_str.startswith("LicenseRef-"):
        return license
    license_id = license_str[:-1]
    if license_id.endswith(("-only", "-or-later")):
        return license
    else:
        return LicenseSymbol(license_id)


def get_license_symbols(pkg_license: str) -> set[BaseSymbol]:
    """Extract all license symbols from a license string

    This function may raise an Exception if it's not possible to derive any meaning from the input string.
    This may be due to being unable to parse the string at all or if a license exception in the string is not a valid
    SPDX license exception identifier.
    """
    licensing = get_spdx_licensing()
    if (license_expression := licensing.parse(pkg_license, strict=True)) is None:
        raise ValueError("Empty license string")
    license_symbols: set[BaseSymbol] = set()
    for symbol in license_expression.symbols:
        if isinstance(symbol, LicenseWithExceptionSymbol):
            license, exception = list(symbol.decompose())
            license = strip_plus_from_license(license)
            license_symbols.add(LicenseWithExceptionSymbol(license, exception))
        else:
            license_symbols.add(strip_plus_from_license(symbol))
    return license_symbols


def get_common_spdx_license_identifiers() -> set[BaseSymbol]:
    """Get all common license identifiers (those that are provided system-wide and can be shared)"""
    common: list[BaseSymbol] = [
        LicenseSymbol(f"{x.stem}") for x in sorted(Path("/usr/share/licenses/spdx/").glob("*.txt")) if x.is_file()
    ]
    return set(common)


def get_common_spdx_license_exceptions() -> set[BaseSymbol]:
    """Get all common license exceptions (those that are provided system-wide and can be shared)"""
    common: list[BaseSymbol] = [
        LicenseSymbol(f"{x.stem}", is_exception=True)
        for x in sorted(Path("/usr/share/licenses/spdx/exceptions/").glob("*.txt"))
        if x.is_file()
    ]
    return set(common)


def get_known_spdx_license_identifiers() -> set[BaseSymbol]:
    """Get all known SPDX license identifiers"""
    all_spdx_licenses: list[BaseSymbol] = []

    with open("/usr/share/licenses/known_spdx_license_identifiers.txt") as file:
        while line := file.readline():
            all_spdx_licenses.append(LicenseSymbol(line.rstrip("\n")))

    return set(all_spdx_licenses)


def get_known_spdx_license_exceptions() -> set[BaseSymbol]:
    """Get all known SPDX license exceptions"""
    all_spdx_licenses: list[BaseSymbol] = []

    with open("/usr/share/licenses/known_spdx_license_exceptions.txt") as file:
        while line := file.readline():
            all_spdx_licenses.append(LicenseSymbol(line.rstrip("\n"), is_exception=True))

    return set(all_spdx_licenses)


def get_uncommon_license_symbols(
    licenses: set[BaseSymbol],
    known_licenses: set[BaseSymbol],
    known_exceptions: set[BaseSymbol],
    common_licenses: set[BaseSymbol],
    common_exceptions: set[BaseSymbol],
) -> set[BaseSymbol]:
    """Get the set of all uncommon licenses from a set of licenses

    This function compares against all uncommon, but known SPDX license symbols (those that require a custom file) or
    those prefixed with "LicenseRef-".
    When encountering LicenseWithExceptionSymbols, both the LicenseSymbol and the exception are checked.
    """
    uncommon_licenses = known_licenses - common_licenses
    uncommon_exceptions = known_exceptions - common_exceptions

    uncommon_symbols: list[BaseSymbol] = []
    for symbol in licenses:
        # if the symbol also carries an exception match license symbol and exception symbol separately
        if isinstance(symbol, LicenseWithExceptionSymbol):
            license, exception = list(symbol.decompose())
            if license in uncommon_licenses or str(license).startswith("LicenseRef-"):
                uncommon_symbols.append(license)
            if exception in uncommon_exceptions or str(exception).startswith("LicenseRef-"):
                uncommon_symbols.append(exception)
        else:
            if symbol in uncommon_licenses or str(list(symbol.decompose())[0]).startswith("LicenseRef-"):
                uncommon_symbols.append(symbol)

    return set(uncommon_symbols)


def get_unknown_license_symbols(
    licenses: set[BaseSymbol], known_licenses: set[BaseSymbol], known_exceptions: set[BaseSymbol]
) -> set[BaseSymbol]:
    """Get the set of all unknown license symbols from a set of licenses

    This function also evaluates LicenseWithExceptionSymbols and considers licenses and exceptions separately.
    """
    unknown_symbols: list[BaseSymbol] = []
    for symbol in licenses:
        # if the symbol also carries an exception match license symbol and exception symbol separately
        if isinstance(symbol, LicenseWithExceptionSymbol):
            license, exception = list(symbol.decompose())
            if license not in known_licenses:
                unknown_symbols.append(symbol)
            if exception not in known_exceptions:
                unknown_symbols.append(symbol)
        else:
            if symbol not in known_licenses:
                unknown_symbols.append(symbol)

    return set(unknown_symbols)


def get_symlink_target(member: TarInfo) -> str:
    """Return the symlink target of a TarInfo

    Absolute symlink targets will be returned as relative Path strings, as we are using them to search for files in
    TarFile objects (and all their members are relative).

    :raises: ValueError if the link target has an invalid amount of upward change dirs.
    """
    symlink_target: str
    if Path(member.linkname).is_absolute():
        symlink_target = str(Path(member.linkname.lstrip("/")))
    else:
        parents = Path(member.linkname).parts.count("..")
        reduce_name_by = -1 - parents

        # if we change dir upwards from the file name too often we may escape the original name and that's not valid
        if (len(Path(member.name).parts) + reduce_name_by) < 0:
            raise ValueError(f"Can not change dir upwards: {member.name} -> {member.linkname}")

        base_parts = Path(member.name).parts[0:reduce_name_by]
        add_parts = tuple(filter(lambda x: x != "..", Path(member.linkname).parts))
        symlink_target = str(Path("/".join(base_parts + add_parts)))

    return symlink_target


def package_license_files(tar: TarFile | None, pkgname: str) -> tuple[dict[str, bool], str | None]:
    """Return the license files referenced in a package and whether the license dir is a symlink"""
    license_dir_symlink = None
    files: dict[str, bool] = {}

    if not tar:
        return (files, license_dir_symlink)

    for member in tar.getmembers():
        # check if entire /usr/share/license/{pkgname}/ dir is a symlink
        if member.issym() and member.name == f"usr/share/licenses/{pkgname}":
            try:
                license_dir_symlink = get_symlink_target(member)
            except ValueError:
                continue
            else:
                break
        # the license is a file
        if (
            member.isfile()
            and not member.issym()
            and member.name.startswith(f"usr/share/licenses/{pkgname}/")
            and member.name != f"usr/share/licenses/{pkgname}"
        ):
            files[member.name] = True
        # the license is a symlink, we'll check later
        if member.issym() and member.name.startswith(f"usr/share/licenses/{pkgname}/"):
            try:
                files[get_symlink_target(member)] = False
            except ValueError:
                continue

    # check existence of all symlinked files
    for file, exists in [(file, exists) for (file, exists) in files.items() if not exists]:
        for member in tar.getmembers():
            if member.isfile() and not member.issym() and member.name == file:
                files[file] = True

    # if license dir is a symlink, add all files below the targeted license dir to files dict
    if license_dir_symlink:
        for member in tar.getmembers():
            if member.isfile() and not member.issym() and member.name.startswith(license_dir_symlink):
                files[member.name] = True

    return (files, license_dir_symlink)


class package(TarballRule):
    name = "licensepkg"
    description = "Verifies license is included in a package file"

    def analyze(self, pkginfo: PacmanPackage, tar: TarFile | None) -> None:
        # return early, as we do not check debug packages
        if is_debug(pkginfo):
            return

        # error and return early if there is no license
        if not pkginfo.get("license"):
            self.errors.append(("missing-license", ()))
            return

        # get canonicalized form of all license strings (and add errors for them)
        for license in pkginfo["license"]:
            try:
                canonicalized_license = get_license_canonicalized(license)
            except Exception:
                # we pass here, as we call get_license_symbols() later, which will raise as well
                pass
            else:
                if license != canonicalized_license:
                    self.errors.append(("license-statement-formatting %s %s", (license, canonicalized_license)))

        # get a combined set of all license symbols from all license strings
        # the license symbols may be of type LicenseSymbol (license) or LicenseWithExceptionSymbol (license + exception)
        license_symbols: set[BaseSymbol] = set()
        for license in pkginfo["license"]:
            try:
                new_license_symbols = get_license_symbols(license)
            except Exception:
                self.errors.append(("invalid-license-string %s", (license,)))
            else:
                license_symbols.update(new_license_symbols)

        known_licenses = get_known_spdx_license_identifiers()
        known_exceptions = get_known_spdx_license_exceptions()

        # check if any license (ignoring exception) symbols are unknown
        # (and add errors for them, if they are not prefixed with LicenseRef-)
        for license in get_unknown_license_symbols(license_symbols, known_licenses, known_exceptions):
            if not str(license).startswith("LicenseRef-"):
                self.errors.append(("unknown-spdx-license-identifier %s", (str(license),)))

        common_licenses = get_common_spdx_license_identifiers()
        common_exceptions = get_common_spdx_license_exceptions()

        # check whether there is a discrepancy between uncommon license symbols and license files found in the package
        uncommon_license_symbols = get_uncommon_license_symbols(
            license_symbols, known_licenses, known_exceptions, common_licenses, common_exceptions
        )
        if len(uncommon_license_symbols) == 0:
            return

        (pkg_licenses, license_dir_symlink) = package_license_files(tar, pkginfo["name"])
        licenses_in_pkg = len([(file, exists) for (file, exists) in pkg_licenses.items() if exists])
        licenses_outside_pkg = len([(file, exists) for (file, exists) in pkg_licenses.items() if not exists])

        # there are not enough license files in the package's license directory
        if licenses_in_pkg < len(uncommon_license_symbols) and licenses_outside_pkg == 0 and not license_dir_symlink:
            self.errors.append(
                (
                    "license-file-missing %s %s %s",
                    (
                        str(", ".join([str(id) for id in list(uncommon_license_symbols)])),
                        pkginfo["name"],
                        f"{licenses_in_pkg}/{len(uncommon_license_symbols)}",
                    ),
                )
            )

        # there are symlinks for license files that point to files in external packages
        if licenses_in_pkg == 0 and licenses_outside_pkg > 0 and not license_dir_symlink:
            outbound_licenses = [license for (license, exists) in pkg_licenses.items() if not exists]
            self.warnings.append(("license-file-in-external-pkg %s", (", ".join(outbound_licenses),)))

            for other_pkg in [load_from_db(name) for name in pkginfo["depends"]]:
                for outbound_license in outbound_licenses:
                    if outbound_license in other_pkg["files"]:
                        pkg_licenses[outbound_license] = True
                        # if there are no license files left to check, break early
                        if not [license for (license, exists) in pkg_licenses.items() if not exists]:
                            break

            outbound_licenses = [license for (license, exists) in pkg_licenses.items() if not exists]
            # there are symlinks to license files in other pkgs, but those files are missing
            # (or the package in question is not a dependency)
            for outbound_license in outbound_licenses:
                self.errors.append(
                    (
                        "license-file-missing-in-other-pkg %s",
                        (outbound_license,),
                    )
                )

        # the license dir is a symlink which points to another package
        if licenses_in_pkg == 0 and licenses_outside_pkg == 0 and license_dir_symlink:
            self.warnings.append(("license-dir-in-external-pkg %s", (license_dir_symlink,)))

            # try to figure out which other package contains the actual files
            other_pkg = (
                list(Path(license_dir_symlink.removeprefix("usr/share/licenses/")).parts)[0]
                if license_dir_symlink.startswith("usr/share/licenses/")
                else None
            )
            if other_pkg:
                if other_pkg not in pkginfo["depends"]:
                    self.errors.append(
                        (
                            "license-dir-target-pkg-not-in-depends %s %s %s",
                            (
                                other_pkg,
                                str(", ".join([str(id) for id in list(uncommon_license_symbols)])),
                                f"0/{len(uncommon_license_symbols)}",
                            ),
                        )
                    )
                    return

                other_pkg_info = load_from_db(other_pkg)
                for file, _, _ in other_pkg_info["files"]:
                    if file.startswith(license_dir_symlink) and file != license_dir_symlink:
                        licenses_outside_pkg += 1
            else:
                for other_pkg_info in [load_from_db(name) for name in pkginfo["depends"]]:
                    for file, _, _ in other_pkg_info["files"]:
                        if file.startswith(license_dir_symlink) and file != license_dir_symlink:
                            licenses_outside_pkg += 1

            # there are still some files missing
            if licenses_outside_pkg < len(uncommon_license_symbols):
                self.errors.append(
                    (
                        "license-dir-is-symlink-and-license-files-missing %s %s %s",
                        (
                            str(", ".join([str(id) for id in list(uncommon_license_symbols)])),
                            license_dir_symlink,
                            f"{licenses_outside_pkg}/{len(uncommon_license_symbols)}",
                        ),
                    )
                )
