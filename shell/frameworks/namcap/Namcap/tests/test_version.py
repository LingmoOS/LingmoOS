# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import unittest
import re
import Namcap.version


class VersionTests(unittest.TestCase):
    def test_manpage(self):
        """Test that the manpage and program has the same version."""
        here = os.path.dirname(os.path.realpath(__file__))
        with open(os.path.join(here, "..", "..", "namcap.1")) as f:
            first_line = f.readline()
        match = re.search(r'"namcap (.*?)"', first_line)
        assert match is not None
        self.assertEqual(match.group(1), Namcap.version.get_version())
