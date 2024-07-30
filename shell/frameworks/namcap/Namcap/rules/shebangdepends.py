# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Checks dependencies on programs specified in shebangs."""

import shutil
import Namcap.package
from Namcap.util import is_script, script_type
from Namcap.ruleclass import TarballRule


def scanshebangs(fileobj, filename, scripts):
    """
    Scan a file for shebang and stores the interpreter name.

    Stores
      scripts -- a dictionary { program => set(scripts) }
    """

    # test magic bytes
    if not is_script(fileobj):
        return
    # process shebang line
    cmd = script_type(fileobj)
    if cmd is not None:
        assert isinstance(cmd, str)
        scripts.setdefault(cmd, set()).add(filename)


def findowners(scriptlist):
    """
    Find owners for executables.

    Returns:
      pkglist -- a dictionary { package => set(programs) }
      orphans -- a set of scripts not found
    """

    pkglist: dict[str, set[str]] = {}
    scriptfound = set()

    for s in scriptlist:
        out = shutil.which(s)
        if not out:
            continue

        # strip leading slash
        scriptpath = out.lstrip("/")
        for pkg in Namcap.package.get_installed_packages():
            pkg_files = [fname for fname, fsize, fmode in pkg.files]
            if scriptpath in pkg_files:
                pkglist.setdefault(pkg.name, set()).add(s)
                scriptfound.add(s)

    orphans = list(set(scriptlist) - scriptfound)
    return pkglist, orphans


class ShebangDependsRule(TarballRule):
    name = "shebangdepends"
    description = "Checks dependencies semi-smartly."

    def analyze(self, pkginfo, tar):
        scriptlist: dict[str, set[str]] = {}

        for entry in tar:
            if not entry.isfile():
                continue
            f = tar.extractfile(entry)
            scanshebangs(f, entry.name, scriptlist)
            f.close()

        # find packages owning interpreters
        pkglist, orphans = findowners(scriptlist)
        for dep, progs in pkglist.items():
            if not isinstance(progs, set):
                continue
            needing = set().union(*[scriptlist[prog] for prog in progs])
            reasons = pkginfo.detected_deps.setdefault(dep, [])
            reasons.append(("programs-needed %s %s", (str(list(progs)), str(list(needing)))))

        # Do the script handling stuff
        for i, v in scriptlist.items():
            files = list(v)
            self.infos.append(("script-link-detected %s in %s", (i, str(files))))

        # Handle "no package associated" errors
        self.warnings.extend([("library-no-package-associated %s %s", (i, str(list(scriptlist[i])))) for i in orphans])

        # Check for packages in testing
        for i in scriptlist:
            p = Namcap.package.load_testing_package(i)
            q = Namcap.package.load_from_db(i)
            if p is not None and q is not None and p["version"] == q["version"]:
                self.warnings.append(("dependency-is-testing-release %s", (i,)))
