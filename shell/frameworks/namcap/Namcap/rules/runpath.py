# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.util import is_elf
from Namcap.ruleclass import TarballRule

from elftools.elf.elffile import ELFFile
from elftools.elf.dynamic import DynamicSection

allowed = ["/usr/lib", "/usr/lib32", "/lib", "$ORIGIN", "${ORIGIN}"]
allowed_toplevels = [s + "/" for s in allowed]
warn = ["/usr/local/lib"]


def get_runpaths(fileobj):
    elffile = ELFFile(fileobj)
    for section in elffile.iter_sections():
        if not isinstance(section, DynamicSection):
            continue
        for tag in section.iter_tags("DT_RUNPATH"):
            for path in tag.runpath.split(":"):
                yield path


class package(TarballRule):
    name = "runpath"
    description = "Verifies if RUNPATH is secure"

    def analyze(self, pkginfo, tar):
        for entry in tar:
            if not entry.isfile():
                continue

            fileobj = tar.extractfile(entry)
            if not is_elf(fileobj):
                continue

            for path in get_runpaths(fileobj):
                path_ok = path in allowed
                if any(path.startswith(tl) for tl in allowed_toplevels):
                    path_ok = True

                if not path_ok:
                    if path in warn:
                        self.warnings.append(("insecure-runpath %s %s", (path, entry.name)))
                    else:
                        self.errors.append(("insecure-runpath %s %s", (path, entry.name)))
