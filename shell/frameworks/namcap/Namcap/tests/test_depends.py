# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import unittest
import Namcap.depends
import Namcap.package


class DependsTests(unittest.TestCase):
    def setUp(self):
        self.pkginfo = Namcap.package.PacmanPackage({"name": "package"})

    def test_missing(self):
        self.pkginfo.detected_deps = {"pkg1": []}
        e, w, i = Namcap.depends.analyze_depends(self.pkginfo)
        expected_e = [("dependency-detected-not-included %s (%s)", ("pkg1", ""))]
        self.assertEqual(e, expected_e)
        self.assertEqual(w, [])
        self.assertEqual(i, [("depends-by-namcap-sight depends=(%s)", ("pkg1",))])

    def test_unneeded(self):
        self.pkginfo["depends"] = {"pkg1": []}
        e, w, i = Namcap.depends.analyze_depends(self.pkginfo)
        expected_w = [("dependency-not-needed %s", ("pkg1",))]
        self.assertEqual(e, [])
        self.assertEqual(w, expected_w)
        self.assertEqual(i, [("depends-by-namcap-sight depends=(%s)", ("",))])

    def test_satisfied(self):
        # false negative test
        self.pkginfo["depends"] = {"readline": []}
        self.pkginfo.detected_deps = {"glibc": [], "readline": []}
        e, w, i = Namcap.depends.analyze_depends(self.pkginfo)
        expected_w = [("dependency-implicitly-satisfied %s (%s)", ("glibc", ""))]
        self.assertEqual(e, [])
        self.assertEqual(w, expected_w)
        # info is verbose and beyond scope, skip it

    def test_satisfied2(self):
        # false positive test
        self.pkginfo["depends"] = {"pyalpm": [], "python": []}
        self.pkginfo.detected_deps = {"pyalpm": [], "python": []}
        e, w, i = Namcap.depends.analyze_depends(self.pkginfo)
        self.assertEqual(e, [])
        self.assertEqual(w, [])
        # info is verbose and beyond scope, skip it
