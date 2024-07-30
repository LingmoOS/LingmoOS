#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

import argparse
import sys

from kapidox import depdiagram

DESCRIPTION = """\
"""


def main():
    parser = argparse.ArgumentParser(description=DESCRIPTION)

    parser.add_argument("-o", "--output", dest="output", default="-",
                        help="Output to FILE", metavar="FILE")

    parser.add_argument("--qt", dest="qt", action="store_true",
                        help="Show Qt libraries")

    parser.add_argument("--detailed", dest="detailed", action="store_true",
                        help="Show targets within frameworks")

    parser.add_argument("--framework", dest="framework",
                        help="Only show dependencies of framework FRAMEWORK", metavar="FRAMEWORK")

    parser.add_argument("dot_files", nargs="+")

    args = parser.parse_args()

    if args.output == "-":
        out = sys.stdout
    else:
        out = open(args.output, "w")

    if depdiagram.generate(out, args.dot_files, framework=args.framework, with_qt=args.qt, detailed=args.detailed):
        return 0
    else:
        return 1


if __name__ == "__main__":
    sys.exit(main())

# vi: ts=4 sw=4 et
