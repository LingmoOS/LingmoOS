#!/usr/bin/env python3
# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import os
import sys
import tarfile

import Namcap.depends
from Namcap.package import load_from_tarball, PacmanPackage
import Namcap.rules
import Namcap.tags
import Namcap.version


# Functions
def get_modules():
    """Return all possible modules (rules)"""
    return Namcap.rules.all_rules


def get_enabled_modules():
    """Return modules (rules) that should be used by default"""
    return dict(filter(lambda x: x[1].enable, get_modules().items()))


def open_package(filename):
    try:
        tar = tarfile.open(filename, "r")
        if ".PKGINFO" not in tar.getnames():
            tar.close()
            return None
    except IOError:
        if tar:
            tar.close()
        return None
    return tar


def show_messages(name, key, messages):
    colored_key = {
        "E": "\033[91mE\033[00m",
        "W": "\033[93mW\033[00m",
        "I": "\033[92mI\033[00m",
    }
    for msg in messages:
        if sys.stdout.isatty():
            print("%s %s: %s" % (name, colored_key[key], Namcap.tags.format_message(msg)))
        else:
            print("%s %s: %s" % (name, key, Namcap.tags.format_message(msg)))


def process_realpackage(package, modules):
    """Runs namcap checks over a package tarball"""
    pkgtar = open_package(package)

    if not pkgtar:
        print("Error: %s is empty or is not a valid package" % package)
        return 1

    pkginfo: PacmanPackage | None = load_from_tarball(package)
    if pkginfo is None:
        print(f"Error: Loading package from {package} failed")
        return 1

    # Loop through each one, load them apply if possible
    for i in modules:
        rule = get_modules()[i]()

        if isinstance(rule, Namcap.ruleclass.PkgInfoRule):
            rule.analyze(pkginfo, None)
        elif isinstance(rule, Namcap.ruleclass.PkgbuildRule):
            pass
        elif isinstance(rule, Namcap.ruleclass.TarballRule):
            rule.analyze(pkginfo, pkgtar)
        else:
            show_messages(pkginfo["name"], "E", [("error-running-rule %s", i)])

        # Output the three types of messages
        show_messages(pkginfo["name"], "E", rule.errors)
        show_messages(pkginfo["name"], "W", rule.warnings)
        if info_reporting:
            show_messages(pkginfo["name"], "I", rule.infos)

    # dependency analysis
    errs, warns, infos = Namcap.depends.analyze_depends(pkginfo)
    show_messages(pkginfo["name"], "E", errs)
    show_messages(pkginfo["name"], "W", warns)
    if info_reporting:
        show_messages(pkginfo["name"], "I", infos)


def process_pkginfo(pkginfo, modules):
    """Runs namcap checks of a single, non-split PacmanPackage object"""
    for i in modules:
        rule = get_modules()[i]()
        if isinstance(rule, Namcap.ruleclass.PkgInfoRule):
            rule.analyze(pkginfo, None)

        # Output the messages
        if "base" in pkginfo:
            name = "PKGBUILD (" + pkginfo["base"] + ")"
        else:
            name = "PKGBUILD (" + pkginfo["name"] + ")"
        show_messages(name, "E", rule.errors)
        show_messages(name, "W", rule.warnings)
        if info_reporting:
            show_messages(name, "I", rule.infos)


def process_pkgbuild(package, modules):
    """Runs namcap checks over a PKGBUILD"""
    # We might want to do some verifying in here... but really... isn't that
    # what pacman.load is for?
    pkginfo = Namcap.package.load_from_pkgbuild(package)

    if pkginfo is None:
        print("Error: %s is not a valid PKGBUILD" % package)
        return 1

    # apply global PKGBUILD rules
    for i in modules:
        rule = get_modules()[i]()
        if isinstance(rule, Namcap.ruleclass.PkgbuildRule):
            rule.analyze(pkginfo, package)
        # Output the messages
        if "base" in pkginfo:
            name = "PKGBUILD (" + pkginfo["base"] + ")"
        else:
            name = "PKGBUILD (" + pkginfo["name"] + ")"
        show_messages(name, "E", rule.errors)
        show_messages(name, "W", rule.warnings)
        if info_reporting:
            show_messages(name, "I", rule.infos)
    # apply per pkginfo rule
    for subpkg in pkginfo.subpackages if pkginfo.is_split else [pkginfo]:
        process_pkginfo(subpkg, modules)


# Main
modules = get_modules()
# Let's handle those options!
version = Namcap.version.get_version()

parser = argparse.ArgumentParser()
parser.add_argument("-L", "--list", action="store_true", help="List available rules")
parser.add_argument("-i", "--info", action="store_true", help="Prints information (debug) responses from rules")
parser.add_argument(
    "-m", "--machine-readable", action="store_true", help="Makes the output parseable (machine-readable)"
)
parser.add_argument("-t", "--tags", action="store", help="Use a custom tag file")
parser.add_argument("packages", nargs="*")
pargroup = parser.add_mutually_exclusive_group()
pargroup.add_argument(
    "-e",
    "--exclude",
    action="store",
    metavar="RULELIST",
    help="Don't apply RULELIST rules to the package (comma-separated)",
)
pargroup.add_argument(
    "-r",
    "--rules",
    action="store",
    metavar="RULELIST",
    help="Only apply RULELIST rules to the package (comma-separated)",
)
parser.add_argument("-v", "--version", action="version", version=version)
args = parser.parse_args()

if args.list:
    print("-" * 20 + " Namcap rule list " + "-" * 20)
    print(modules)
    for j in sorted(modules):
        print("%-20s: %s" % (j, modules[j].description))
    parser.exit(0)

if len(args.packages) == 0:
    print("Missing required argument packages", file=sys.stderr)
    parser.exit(2)

info_reporting = args.info
machine_readable = args.machine_readable
filename = args.tags
packages = args.packages

active_modules = {}

if args.rules:
    for rule in args.rules.split(","):
        if rule in modules:
            active_modules[rule] = modules[rule]
        else:
            print(f"Error: Rule '{rule}' does not exist")
            parser.exit(2)

if args.exclude:
    for rule in args.exclude.split(","):
        active_modules.update(modules)
        if rule in modules:
            active_modules.pop(rule)
        else:
            print(f"Error: Rule '{rule}' does not exist")
            parser.exit(2)

Namcap.tags.load_tags(filename=filename, machine=machine_readable)

# No rules selected?  Then use default selection
if len(active_modules) == 0:
    active_modules = get_enabled_modules()

# Go through each package, get the info, and apply the rules
for package in packages:
    if not os.access(package, os.R_OK):
        print("Error: Problem reading %s" % package)
        parser.print_usage()

    if os.path.isfile(package) and tarfile.is_tarfile(package):
        process_realpackage(package, active_modules)
    elif "PKGBUILD" in package:
        process_pkgbuild(package, active_modules)
    else:
        print("Error: %s not package or PKGBUILD" % package)
