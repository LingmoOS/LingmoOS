#!/usr/bin/env python3
#
# Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


import json
import os
import sys

def get_language_list():
    """Parse language list and returns locale list."""
    path = "resources/languages.json"
    with open(path) as fh:
        obj = json.load(fh)
    if obj:
        return [lang["locale"] for lang in obj]
    else:
        return []

def generate_ts(ts_path):
    """Generate new ts file.

    Scan source files in `ui` module recursively.
    And save result to |ts_path|.
    """
   
    paths = (
        "prepare",
        "server",
        "installer",
        "gltest",
        "test_installer"
    )

    # Add -I. option to solve namespace error
    cmd = " ".join((
        "lupdate -recursive -I.",
        " ".join(paths),
        "-ts", ts_path,
        ))
    os.system(cmd)

def main():
    # Make sure that PWD is root of source repo.
    if not os.path.isdir("translations"):
        print("Run script in parent folder of `translations`")
        sys.exit(1)

    default_ts = "translations/deepin-graphics-driver-manager.ts"
    generate_ts(default_ts)

if __name__ == "__main__":
    main()
