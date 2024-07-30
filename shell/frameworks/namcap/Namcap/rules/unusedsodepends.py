# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import re
import subprocess
import tempfile
from Namcap.util import is_elf
from Namcap.ruleclass import TarballRule

libre = re.compile(r"^\t(/.*)")
lddfail = re.compile(r"^\tnot a dynamic executable")


def get_unused_sodepends(filename):
    p = subprocess.Popen(
        ["ldd", "-r", "-u", filename], env={"LANG": "C"}, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    var = p.communicate()
    if p.returncode == 0:
        return
    for j in var[0].decode("ascii").splitlines():
        # Don't raise an error, as the executable might be a valid ELF file,
        # just not a dynamically linked one
        n = lddfail.search(j)
        if n is not None:
            return

        n = libre.search(j)
        if n is not None:
            yield n.group(1)


class package(TarballRule):
    name = "unusedsodepends"
    description = "Checks for unused dependencies caused by linked shared libraries"

    def analyze(self, pkginfo, tar):
        for entry in tar:
            if not entry.isfile():
                continue

            # is it an ELF file ?
            f = tar.extractfile(entry)
            if not is_elf(f):
                f.close()
                continue
            elf = f.read()
            f.close()

            # write it to a temporary file
            f = tempfile.NamedTemporaryFile(delete=False)
            f.write(elf)
            f.close()

            os.chmod(f.name, 0o755)

            for lib in get_unused_sodepends(f.name):
                self.warnings.append(("unused-sodepend %s %s", (lib, entry.name)))

            os.unlink(f.name)
