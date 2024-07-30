# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from collections import defaultdict
import re
import Namcap.package
from Namcap.ruleclass import TarballRule
from Namcap.util import is_elf

qml_path = "usr/lib/qt6/qml/"


def finddepends(modules):
    """
    Find packages owning a list of modules

    Returns:
      dependlist -- a dictionary { package => set(modules) }
      orphans -- the list of modules without owners
    """
    dependlist = defaultdict(set)
    foundlibs = set()

    for pkg in Namcap.package.get_installed_packages():
        for j, fsize, fmode in pkg.files:
            if j.startswith(qml_path) and j.endswith("/qmldir"):
                k = j.replace(qml_path, "").replace("/qmldir", "").replace("/", ".")
                if k in modules:
                    dependlist[pkg.name].add(k)
                    foundlibs.add(k)

    orphans = list(set(modules) - foundlibs)
    return dependlist, orphans


def get_imports(string, filename, modules):
    """
    Extract all QML imports from a file
    """
    import_pattern = r"^[ ]?import [\w.]+"
    imports = re.findall(import_pattern, string, re.MULTILINE)

    for m in imports:
        m = m.strip().replace("import ", "")
        modules[m].add(filename)
    return


class QmlDependencyRule(TarballRule):
    name = "qmldepends"
    description = "Checks QML dependencies"

    def analyze(self, pkginfo, tar):
        modules: dict[str, set[str]] = defaultdict(set)
        included_modules = []

        for entry in tar:
            if not entry.isfile():
                continue
            if entry.name.startswith(qml_path) and entry.name.endswith("/qmldir"):
                included_modules += [entry.name.replace(qml_path, "").replace("/qmldir", "").replace("/", ".")]
                continue
            if not entry.name.endswith(".qml") and not any(entry.name.startswith(d) for d in ["usr/bin", "usr/lib"]):
                continue
            f = tar.extractfile(entry)
            elf = is_elf(f)
            if not entry.name.endswith(".qml") and not elf:
                continue
            s = f.read().decode(errors="ignore")
            if elf and "libQt6Qml.so" not in s:
                # Does not embed QML, prevent false positives
                continue
            get_imports(s, entry.name, modules)
            f.close()

        for m in included_modules:
            modules.pop(m, None)

        dependlist, orphans = finddepends(modules)
        liblist = modules

        # Handle "no package associated" errors
        self.warnings.extend([("qml-module-no-package-associated %s %s", (i, str(list(liblist[i])))) for i in orphans])

        # Print QML module deps
        for pkg, libraries in dependlist.items():
            if isinstance(libraries, set):
                files = list(libraries)
                needing = set().union(*[liblist[lib] for lib in libraries])
                reasons = pkginfo.detected_deps.setdefault(pkg, [])
                reasons.append(("qml-modules-needed %s %s", (str(files), str(list(needing)))))
                self.infos.append(("qml-module-dependence %s in %s", (pkg, str(files))))
