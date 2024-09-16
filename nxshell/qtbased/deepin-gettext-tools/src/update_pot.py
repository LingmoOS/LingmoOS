#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2015 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

import argparse
import os
import re
import subprocess

try:
    from configparser import RawConfigParser as ConfigParser
except ImportError:
    from ConfigParser import RawConfigParser as ConfigParser

def update_pot(config_path):
    # Read config options.
    config_parser = ConfigParser()
    config_parser.read(config_path)
    config_dir = os.path.dirname(os.path.realpath(config_path))
    os.chdir(config_dir)
    project_name = config_parser.get("locale", "project_name")

    # Source dirs are separated by blank chars
    source_dirs = re.split('\s+', config_parser.get("locale", "source_dir"))

    locale_dir = os.path.abspath(config_parser.get("locale", "locale_dir"))
    if not os.path.exists(locale_dir):
        os.makedirs(locale_dir)

    pot_filepath = os.path.join(locale_dir, project_name + ".pot")
    if os.path.exists(pot_filepath):
        os.remove(pot_filepath)

    py_source_files = []
    go_source_files = []
    for source_dir in source_dirs:
        for root, dirs, files in os.walk(source_dir):
            for f in files:
                if f.startswith("."):
                    continue
                if f.endswith(".py"):
                    py_source_files.append(os.path.join(root, f))
                elif f.endswith(".go"):
                    go_source_files.append(os.path.join(root, f))

    if len(py_source_files) > 0:
        gen_pot(pot_filepath, ['-k_'], py_source_files)

    if len(go_source_files) > 0:
        gen_pot(pot_filepath, ['-kTr', '-C', '--from-code=utf-8'], go_source_files)

def gen_pot(output, args, source_files):
    callargs = ['xgettext', '-F', '-o', output]
    if os.path.exists(output):
        # join messages with existing file
        callargs.append('-j')
    callargs.extend(args)
    #print callargs
    callargs.extend(source_files)
    subprocess.call(callargs)

def valid_path(string):
    """
    check if the path entered is a valid one
    """
    if not os.path.isfile(string):
        msg = "%s is not a valid file" % string
        raise argparse.ArgumentTypeError(msg)
    return string

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Scan msgid and generate pot file according to the ini config file',
        epilog='A domain.pot file will be generated in the locale directory')
    parser.add_argument('file',metavar='file',
        type=valid_path,
        help='A valid ini config path, full or local.')

    args = parser.parse_args()
    update_pot(args.file)

