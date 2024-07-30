#!/usr/bin/env python3
"""
SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
"""
import os
import sys
import mmap
"""
This script is just a convenient way to get a list of qml objects so that they can be pasted into the qmldir file
"""

INPUT_DIR = "./"
if len(sys.argv) > 1:
    INPUT_DIR = sys.argv[1]

for dirpath, dirnames, filenames in os.walk(INPUT_DIR):
    for f in filenames:
        # Filter out files
        if not (f.endswith('.qml')):
            continue

        filepath = os.path.join(dirpath, f)

        # Thanks StackOverflow! https://stackoverflow.com/questions/4940032/how-to-search-for-a-string-in-text-files
        with open(filepath, 'rb', 0) as file, mmap.mmap(file.fileno(), 0, access=mmap.ACCESS_READ) as s:
            if s.find(b"pragma Singleton") != -1:
                print("singleton " + f.rstrip('.qml') + " 1.0 " + f)
            else:
                print(f.rstrip('.qml') + " 1.0 " + f)
