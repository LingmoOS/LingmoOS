# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from Namcap.ruleclass import TarballRule
from Namcap.util import is_java


class JavaFiles(TarballRule):
    name = "javafiles"
    description = "Check for existence of Java classes or JARs"

    def analyze(self, pkginfo, tar):
        javas = []
        for entry in tar:
            # is it a regular file ?
            if not entry.isfile():
                continue
            # is it a JAR file ?
            if entry.name.endswith(".jar"):
                javas.append(entry.name)
                # self.infos.append( ('jar-file-found %s', entry.name) )
                continue
            # is it a CLASS file ?
            f = tar.extractfile(entry)
            if is_java(f):
                javas.append(entry.name)
                # self.infos.append( ('java-class-file-found %s', entry.name) )
            f.close()
        if javas:
            reasons = pkginfo.detected_deps.setdefault("java-runtime", [])
            reasons.append(("java-runtime-needed %s", ", ".join(javas)))
