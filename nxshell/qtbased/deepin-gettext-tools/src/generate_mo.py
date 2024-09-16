#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2015 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

import subprocess
import os
import sys
import argparse

try:
    from configparser import RawConfigParser as ConfigParser
except ImportError:
    from ConfigParser import RawConfigParser as ConfigParser

def main(args):
    # Read config options.

    config_path = args.file
    copy = args.copy

    config_parser = ConfigParser()
    config_parser.read(config_path)
    config_dir = os.path.dirname(os.path.realpath(config_path))
    os.chdir(config_dir)
    project_name = config_parser.get("locale", "project_name")
    locale_dir = os.path.abspath(config_parser.get("locale", "locale_dir"))
    mo_locale_dir = os.path.join(locale_dir, "mo")

    for f in os.listdir(locale_dir):
        lang, ext = os.path.splitext(f)
        if ext == ".po":
            mo_dir = os.path.join(mo_locale_dir, lang, "LC_MESSAGES")
            mo_path = os.path.join(mo_dir, "%s.mo" % project_name)
            po_path = os.path.join(locale_dir, f)
            subprocess.call(
                "mkdir -p %s" % mo_dir,
                shell=True
                )

            subprocess.check_call(
                "msgfmt -o %s %s" % (mo_path, po_path),
                shell=True
                )

            if copy:
                subprocess.call(
                    "sudo cp -r %s %s" % (
                        os.path.join(mo_locale_dir, lang),
                        "/usr/share/locale/"),
                    shell=True
                    )
    return 0

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
        description='Scan po files and generate mo files according to the ini config file',
        epilog='A mo folder will be generated in the locale directory')
    parser.add_argument('file',metavar='file',
        type=valid_path,
        help='A valid ini config path, full or local.')
    parser.add_argument('--copy', dest='copy', action='store_true',
        help='Execute "sudo cp *.mo /usr/share/locale"')

    args = parser.parse_args()
    sys.exit(main(args))
