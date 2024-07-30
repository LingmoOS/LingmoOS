# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from types import ModuleType
import Namcap.ruleclass

# Tarball rules
from . import (  # noqa: F401
    anyelf,
    dbus1location,
    elffiles,
    emptydir,
    externalhooks,
    fhs,
    filenames,
    fileownership,
    gnomemime,
    hardlinks,
    hookdepends,
    infodirectory,
    javafiles,
    libtool,
    licensepkg,
    lotsofdocs,
    missingbackups,
    pathdepends,
    perllocal,
    pcdepends,
    permissions,
    py_mtime,
    pydepends,
    qmldepends,
    rpath,
    runpath,
    scrollkeeper,
    shebangdepends,
    sphinxbuildcachefiles,
    sodepends,
    symlink,
    systemdlocation,
    unusedsodepends,
)

# PKGBUILD and metadata rules
from . import (  # noqa: F401
    arrays,
    badbackups,
    carch,
    extravars,
    invalidstartdir,
    makedepends,
    makepkgfunctions,
    missingvars,
    pkginfo,
    pkgnameindesc,
    sfurl,
    splitpkgbuild,
)

all_rules = {}
for name, value in dict(locals()).items():
    if not isinstance(value, ModuleType):
        continue
    if name == "Namcap.ruleclass":
        continue
    for n, v in value.__dict__.items():
        if type(v) == type and issubclass(v, Namcap.ruleclass.AbstractRule) and hasattr(v, "name"):
            all_rules[v.name] = v
