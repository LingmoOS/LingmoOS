# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

from collections import defaultdict
import ast
import importlib
import sys
import sysconfig
import Namcap.package
from Namcap.util import is_script, script_type
from Namcap.ruleclass import TarballRule


def finddepends(pkgname, modules, gir_modules, gir_versions):
    """
    Find packages owning a list of libraries

    Returns:
      dependlist -- a dictionary { package => set(libraries) }
      orphans -- the list of libraries without owners
      gir_dependlist -- a dictionary { package => set(GIR libraries) }
      gir_orphans -- the list of GIR libraries without owners
    """
    python_path = sysconfig.get_path("stdlib", scheme="posix_prefix")
    site_packages_path = sysconfig.get_path("purelib", scheme="posix_prefix")

    dependlist = defaultdict(set)
    gir_dependlist = defaultdict(set)

    knownlibs: dict[str, str] = defaultdict(str)
    missinglibs = set()
    foundlibs = set()
    gir_foundlibs = set()

    for module in modules:
        # Check application-specific python modules
        if importlib.machinery.PathFinder.find_spec(
            module.split(".")[0], ["/usr/lib/" + pkgname, "/usr/share/" + pkgname]
        ):
            dependlist[pkgname].add(module)
            continue

        # Check internal python modules
        if module.split(".")[0] in sys.builtin_module_names or importlib.machinery.PathFinder.find_spec(
            module.split(".")[0], [python_path, python_path + "/lib-dynload"]
        ):
            dependlist["python"].add(module)
            continue

        # Search external python modules
        spec = importlib.machinery.PathFinder.find_spec(module.split(".")[0], [site_packages_path])
        # Search namespaced python module
        if spec and not spec.origin and spec.submodule_search_locations and len(module.split(".")) > 1:
            spec = importlib.machinery.PathFinder.find_spec(module.split(".")[1], spec.submodule_search_locations)
            if spec and not spec.origin and spec.submodule_search_locations and len(module.split(".")) > 2:
                spec = importlib.machinery.PathFinder.find_spec(module.split(".")[2], spec.submodule_search_locations)
        if spec and spec.origin:
            knownlibs[module] = spec.origin
        else:
            missinglibs.add(module)

    for pkg in Namcap.package.get_installed_packages():
        for j, fsize, fmode in pkg.files:
            if j.startswith(site_packages_path[1:]):
                for k, path in knownlibs.items():
                    if j == path[1:]:
                        dependlist[pkg.name].add(k)
                        foundlibs.add(k)

            if j.startswith("usr/lib/girepository-1.0/"):
                for module in gir_modules:
                    gir_module = module.replace("gi.repository.", "")
                    if j.startswith("usr/lib/girepository-1.0/" + gir_module + "-" + gir_versions[gir_module]):
                        gir_dependlist[pkg.name].add(module)
                        gir_foundlibs.add(module)

    orphans = list(set(knownlibs.keys()).union(missinglibs) - foundlibs)
    gir_orphans = list(set(gir_modules.keys()) - gir_foundlibs)
    return dependlist, orphans, gir_dependlist, gir_orphans


def get_imports(fileobj, filename, modules, gir_modules, gir_versions):
    try:
        root = ast.parse(fileobj.read())
    except (SyntaxError, ValueError):
        # ast.parse() uses compile(), which may raise SyntaxError or ValueError
        return

    for node in ast.walk(root):
        if isinstance(node, ast.Import):
            for module in node.names:
                modules[module.name].add(filename)
                if module.name.startswith("gi.repository."):
                    gir_modules[module.name].add(filename)
        elif isinstance(node, ast.ImportFrom):
            if node.module and node.level == 0:
                for submodule in node.names:
                    modules[node.module + "." + submodule.name].add(filename)
                    if node.module == "gi.repository":
                        gir_modules[node.module + "." + submodule.name].add(filename)
        elif (
            isinstance(node, ast.Call)
            and isinstance(node.func, ast.Attribute)
            and isinstance(node.func.value, ast.Name)
            and node.func.value.id == "gi"
            and node.func.attr == "require_version"
        ):
            if hasattr(node.args[0], "value") and hasattr(node.args[1], "value"):
                gir_versions[node.args[0].value] = node.args[1].value
        elif (
            isinstance(node, ast.Call)
            and isinstance(node.func, ast.Attribute)
            and isinstance(node.func.value, ast.Name)
            and node.func.value.id == "gi"
            and node.func.attr == "require_versions"
            and hasattr(node.args[0], "keys")
            and hasattr(node.args[0], "values")
        ):
            for module, version in zip(node.args[0].keys, node.args[0].values):
                if hasattr(module, "value") and hasattr(version, "value"):
                    gir_versions[module.value] = version.value


class PythonDependencyRule(TarballRule):
    name = "pydepends"
    description = "Checks python dependencies"

    def analyze(self, pkginfo, tar):
        modules: dict[str, set[str]] = defaultdict(set)
        gir_modules: dict[str, set[str]] = defaultdict(set)
        gir_versions: dict[str, str] = defaultdict(str)

        for entry in tar:
            if not entry.isfile():
                continue
            f = tar.extractfile(entry)
            if not entry.name.endswith(".py") and not is_script(f):
                continue
            if is_script(f) and script_type(f) not in ["python", "python3"]:
                continue
            get_imports(f, entry.name, modules, gir_modules, gir_versions)
            f.close()

        # If Gdk version is not defined, it should be the same as Gtk version
        if not gir_versions["Gdk"]:
            gir_versions["Gdk"] = gir_versions["Gtk"]
        if not gir_versions["GdkX11"]:
            gir_versions["GdkX11"] = gir_versions["Gtk"]

        dependlist, orphans, gir_dependlist, gir_orphans = finddepends(
            pkginfo["name"], modules, gir_modules, gir_versions
        )
        liblist = modules | gir_modules

        # Handle "no package associated" errors
        self.warnings.extend(
            [("python-module-no-package-associated %s %s", (i, str(list(liblist[i])))) for i in orphans + gir_orphans]
        )

        # Print python module deps
        for pkg, libraries in (dependlist | gir_dependlist).items():
            if isinstance(libraries, set):
                files = list(libraries)
                needing = set().union(*[liblist[lib] for lib in libraries])
                reasons = pkginfo.detected_deps.setdefault(pkg, [])
                reasons.append(("python-modules-needed %s %s", (str(files), str(list(needing)))))
                self.infos.append(("python-module-dependence %s in %s", (pkg, str(files))))
