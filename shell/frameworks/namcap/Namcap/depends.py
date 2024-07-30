# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""Checks dependencies semi-smartly."""

import Namcap.tags
from Namcap import package

from .types import Diagnostic


def single_covered(depend):
    "Returns full coverage tree of one package, with loops broken"
    covered = set()
    todo = set([depend])
    while todo:
        i = todo.pop()
        covered.add(i)
        pac = package.load_from_db(i)
        if pac is None:
            continue
        todo |= set(pac["depends"]) - covered

    return covered - set([depend])


def getcovered(dependlist):
    """
    Returns full coverage tree set, without packages
    from self-loops (iterable of package names)
    """

    covered = set()
    for d in dependlist:
        covered |= single_covered(d)
    return covered


def getprovides(depends):
    provides: dict[str, set[str]] = {}
    for i in depends:
        provides[i] = set()
        pac = package.load_from_db(i)
        if pac is None:
            continue
        if not pac["provides"]:
            continue
        provides[i].update(pac["provides"])
    return provides


def analyze_depends(pkginfo):
    errors: list[Diagnostic] = []
    warnings: list[Diagnostic] = []
    infos: list[Diagnostic] = []

    # compute needed dependencies
    dependlist = set(pkginfo.detected_deps.keys())

    # Find all the covered dependencies from the PKGBUILD
    pkginfo.setdefault("depends", [])
    explicitdepend = set(pkginfo["depends"])
    implicitdepend = getcovered(explicitdepend)

    # Include the optdepends from the PKGBUILD
    pkginfo.setdefault("optdepends", [])
    optdepend = set(pkginfo["optdepends"])
    implicit_optdepend = getcovered(optdepend)

    # Get the provides so we can reference them later
    # smartprovides : depend => (packages provided by depend)
    smartprovides = getprovides(dependlist)

    # The set of all provides for detected dependencies
    allprovides = set()
    for plist in smartprovides.values():
        allprovides |= plist

    # Common deps
    for duplicated_optdepend in explicitdepend & optdepend:
        errors.append(("dependency-duplicated-optdepend %s", (duplicated_optdepend,)))
    for satisfied_optdepend in implicitdepend & optdepend:
        infos.append(("dependency-satisfied-optdepend %s", (satisfied_optdepend,)))

    # Do the actual message outputting stuff
    for i in dependlist:
        # compute dependency reason
        reasons = pkginfo.detected_deps[i]
        reason_strings = [Namcap.tags.format_message(reason) for reason in reasons]
        reason = ", ".join(reason_strings)
        # if the needed package is itself:
        if i == pkginfo["name"]:
            infos.append(("dependency-detected-satisfied %s (%s)", (i, reason)))
            continue
        # if the dependency is satisfied
        if i in explicitdepend or smartprovides[i] & explicitdepend:
            infos.append(("dependency-detected-satisfied %s (%s)", (i, reason)))
            continue
        # still not found, maybe it is specified as optional
        if i in optdepend or smartprovides[i] & optdepend:
            warnings.append(("dependency-detected-but-optional %s (%s)", (i, reason)))
            continue
        # a needed dependency is transitive, it is implicitly satisfied
        if i in implicitdepend or smartprovides[i] & implicitdepend:
            warnings.append(("dependency-implicitly-satisfied %s (%s)", (i, reason)))
            continue
        # maybe, it is pulled as a transitive dependency for an optdepend
        if i in implicit_optdepend or smartprovides[i] & implicit_optdepend:
            warnings.append(("dependency-implicitly-satisfied-optional %s (%s)", (i, reason)))
            continue
        # now i'm pretty sure i didn't find it.
        errors.append(("dependency-detected-not-included %s (%s)", (i, reason)))

    for i in pkginfo["depends"]:
        # multilib packages usually depend on their regular counterparts
        if pkginfo["name"].startswith("lib32-") and i == pkginfo["name"].partition("-")[2]:
            continue
        # a dependency is unneeded if:
        #   it is not in the depends as we see them
        #   it does not pull some needed dependency which provides it
        if i not in dependlist and i not in allprovides:
            warnings.append(("dependency-not-needed %s", (i,)))
    infos.append(("depends-by-namcap-sight depends=(%s)", (" ".join(dependlist),)))

    return errors, warnings, infos
