#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
# SPDX-FileCopyrightText: 2014 Alex Turbov <i.zaufi@gmail.com>
# SPDX-FileCopyrightText: 2016 Olivier Churlaud <olivier@churlaud.com>
#
# SPDX-License-Identifier: BSD-2-Clause

import logging
import codecs
import os
import sys
import time
import datetime

from kapidox import generator, utils, argparserutils, preprocessing, hlfunctions

try:
    from kapidox import depdiagram
    DEPDIAGRAM_AVAILABLE = True
except ImportError:
    DEPDIAGRAM_AVAILABLE = False


def get_identities():
    """Extract account information from the file (if any) specified with
       the `--accountsfile` command-line argument.
       An account file is a whitespace-separated file with a first "column"
       specifying the account name, the last "column" specifying an email
       address, and all the columns in between are a name, e.g.

       adridg Adriaan de Groot groot@kde.org

       This line has three "columns" for the name.
    """
    maintainers = {}

    args = argparserutils.parse_args(DEPDIAGRAM_AVAILABLE)
    if args.accountsfile and os.path.exists(args.accountsfile):
        logging.debug("Found contributor identities file at %s", args.accountsfile)
        with codecs.open(args.accountsfile, 'r', encoding='utf8') as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) >= 3:
                    maintainers[parts[0]] = {
                        'name': ' '.join(parts[1:-1]),
                        'email': parts[-1]
                        }

    else:
        logging.debug("No contributor identities file specified. Without the "
                      "file, maintainer information will be incomplete. For "
                      "generating KDE API documentation you can download the "
                      "KDE accounts file from https://websvn.kde.org/*check"
                      "out*/trunk/kde-common/accounts and specify it with the "
                      "--accountsfile argument.")

    return maintainers


def main():
    kde_copyright = '1996-' + str(datetime.date.today().year) + ' The KDE developers'
    get_maintainers = get_identities

    hlfunctions.do_it(maintainers_fct=get_maintainers,
                      copyright=kde_copyright)


if __name__ == "__main__":
    main()
