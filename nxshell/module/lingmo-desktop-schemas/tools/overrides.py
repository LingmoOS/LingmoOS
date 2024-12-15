#!/usr/bin/env python

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import glob
import argparse
import sys

PY3 = sys.version_info[0] == 3
if PY3:
    from configparser import ConfigParser
else:
    from ConfigParser import ConfigParser

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('arch')
    parser.add_argument('result')
    args = parser.parse_args()

    overrides = glob.glob("overrides/common/*.override")
    overrides.extend(glob.glob("overrides/%s/*.override" % args.arch))

    cf = ConfigParser()
    for override in overrides:
        _override = ConfigParser()
        _override.read(override)
        for section in _override.sections():
            if section not in cf.sections():
                cf.add_section(section)
            for key, value in _override.items(section):
                # value should not be \"\" or empty
                if value == '':
                    raise Exception("%s %s value is empty" % (key, value))
                cf.set(section, key, value)
    
    print("Save override schemas to %s" % args.result)
    cf.write(open(args.result, "w"))
