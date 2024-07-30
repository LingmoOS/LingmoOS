#!/usr/bin/env python
from setuptools import setup, find_packages
import Namcap.version

DATAFILES = [
    ("/usr/share/man/man1", ["namcap.1"]),
    ("/usr/share/namcap", ["namcap-tags", "parsepkgbuild.sh"]),
    ("/usr/share/doc/namcap", ["README.md", "AUTHORS"]),
]

setup(
    name="namcap",
    version=Namcap.version.get_version(),
    description="Pacman package analyzer",
    author="Arch Dev Team",
    author_email="pacman-dev@archlinux.org",
    url="http://www.archlinux.org/",
    py_modules=["namcap"],
    packages=find_packages(),
    scripts=["scripts/namcap", "scripts/parsepkgbuild"],
    test_suite="Namcap.tests",
    data_files=DATAFILES,
    install_requires=["pyalpm", "pyelftools", "license-expression"],
)
