#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

import argparse
import os
import logging
import shutil
import subprocess
import sys
import tempfile
import time

from kapidox import utils

DESCRIPTION = """
Generate Graphviz dot files for one or all frameworks.
"""


def generate_dot(fw_dir, fw_name, output_dir):
    """Calls cmake to generate the dot file for a framework.

    Returns true on success, false on failure"""
    dot_path = os.path.join(output_dir, fw_name + ".dot")
    build_dir = tempfile.mkdtemp(prefix="depdiagram-prepare-build-")
    try:
        ret = subprocess.call(["cmake", fw_dir, "--graphviz={}".format(dot_path)],
                              stdout=open("/dev/null", "w"),
                              cwd=build_dir)
        if ret != 0:
            if os.path.exists(dot_path):
                os.remove(dot_path)
            logging.error("Generating dot file for {} failed.".format(fw_name))
            return False
        # Add a timestamp and version info to help with diagnostics
        with open(dot_path, "a") as f:
            f.write("\n# Generated on {}\n".format(time.ctime()))
            version = utils.get_kapidox_version()
            if version:
                f.write("# By {} {}\n".format(sys.argv[0], version))
    finally:
        shutil.rmtree(build_dir)
    return True


def prepare_one(fw_dir, output_dir):
    fw_name = utils.parse_fancyname(fw_dir)
    if fw_name is None:
        return False

    yaml_path = os.path.join(fw_dir, "metainfo.yaml")
    if not os.path.exists(yaml_path):
        logging.error(f"'{fw_dir}' is not a framework: '{yaml_path}' does not exist.")
        return False

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    if not generate_dot(fw_dir, fw_name, output_dir):
        return False
    shutil.copyfile(yaml_path, os.path.join(output_dir, fw_name + ".yaml"))
    return True


def prepare_all(fw_base_dir, dot_dir):
    """Generate dot files for all frameworks.

    Looks for frameworks in `fw_base_dir`. Output the dot files in sub dirs of
    `dot_dir`.
    """
    lst = os.listdir(fw_base_dir)
    fails = []
    for idx, fw_name in enumerate(lst):
        fw_dir = os.path.join(fw_base_dir, fw_name)
        if not os.path.isdir(fw_dir):
            continue

        progress = int(100 * (idx + 1) / len(lst))
        print(f'{progress}% {fw_name}')
        if not prepare_one(fw_dir, dot_dir):
            fails.append(fw_name)
    return fails


def main():
    parser = argparse.ArgumentParser(description=DESCRIPTION)

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("-s", "--single",
                       help="Generate dot files for the framework stored in DIR",
                       metavar="DIR")
    group.add_argument("-a", "--all",
                       help="Generate dot files for all frameworks whose dir is in BASE_DIR",
                       metavar="BASE_DIR")
    parser.add_argument("dot_dir",
                        help="Destination dir where dot files will be generated")

    args = parser.parse_args()

    dot_dir = os.path.abspath(args.dot_dir)

    if args.single:
        fw_dir = os.path.abspath(args.single)
        if prepare_one(fw_dir, dot_dir):
            return 0
        else:
            return 1
    else:
        fw_base_dir = os.path.abspath(args.all)
        fails = prepare_all(fw_base_dir, dot_dir)
        if fails:
            logging.error("{} framework(s) failed: {}".format(len(fails), ", ".join(fails)))
            return 1
        else:
            return 0


if __name__ == "__main__":
    sys.exit(main())
