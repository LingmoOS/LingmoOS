# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import unittest


def getTestSuite():
    loader = unittest.TestLoader()
    return loader.discover(start_dir=os.path.dirname(__file__))


if __name__ == "__main__":
    runner = unittest.TextTestRunner()
    runner.run(getTestSuite())
