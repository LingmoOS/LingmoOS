# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

"""
This module defines the base classes from which Namcap rules are derived
and how they are meant to be used.
"""

from abc import ABC, abstractmethod
from tarfile import TarFile

from .package import PacmanPackage
from .types import Diagnostic


class AbstractRule(ABC):
    "The parent class of all rules"

    enable: bool = True

    def __init__(self):
        self.errors: list[Diagnostic] = []
        self.warnings: list[Diagnostic] = []
        self.infos: list[Diagnostic] = []


class PkgInfoRule(AbstractRule):
    "The parent class of rules that process package metadata"

    @abstractmethod
    def analyze(self, pkginfo: PacmanPackage, pkgbuild: str | None) -> None: ...


class PkgbuildRule(AbstractRule):
    "The parent class of rules that process PKGBUILDs"

    @abstractmethod
    def analyze(self, pkginfo: PacmanPackage, pkgbuild: str) -> None: ...


class TarballRule(AbstractRule):
    "The parent class of rules that process tarballs"

    @abstractmethod
    def analyze(self, pkginfo: PacmanPackage, tar: TarFile) -> None: ...
