# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2016 Olivier Churlaud <olivier@churlaud.com>
# SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
# SPDX-FileCopyrightText: 2014 Alex Turbov <i.zaufi@gmail.com>
#
# SPDX-License-Identifier: BSD-2-Clause

import logging
import os
import sys
from typing import Any, Dict, Optional

from urllib.request import Request, urlopen
from urllib.error import HTTPError

import yaml

from kapidox import utils
from kapidox.models import Library, Product

__all__ = (
    "create_metainfo",
    "parse_tree")

PLATFORM_ALL = "All"
PLATFORM_UNKNOWN = "UNKNOWN"


## @package kapidox.preprocessing
#
# Preprocessing of the needed information.
#
# The module allow to walk through folders, read metainfo files and create
# products, subgroups and libraries representing the projects.
#

def expand_platform_all(dct, available_platforms):
    """If one of the keys of dct is `PLATFORM_ALL` (or `PLATFORM_UNKNOWN`),
    remove it and add entries for all available platforms to dct

    Args:
        dct: (dictionary) dictionary to expand
        available_platforms: (list of string) name of platforms
    """

    add_all_platforms = False
    if PLATFORM_ALL in dct:
        note = dct[PLATFORM_ALL]
        add_all_platforms = True
        del dct[PLATFORM_ALL]
    if PLATFORM_UNKNOWN in dct:
        add_all_platforms = True
        note = dct[PLATFORM_UNKNOWN]
        del dct[PLATFORM_UNKNOWN]
    if add_all_platforms:
        for platform in available_platforms:
            if platform not in dct:
                dct[platform] = note


def create_metainfo(path) -> Optional[Dict[str, Any]]:
    """Look for a `metadata.yaml` file and create a dictionary out it.

    Args:
        path: (string) the current path to search.
    Returns:
        A dictionary containing all the parsed information, or `None` if it
    did not fulfill some conditions.
    """

    metainfo: Optional[Dict[str, Any]]

    if not os.path.isdir(path):
        return None

    try:
        metainfo_file = os.path.join(path, 'metainfo.yaml')
    except UnicodeDecodeError as e:
        logging.warning('Unusual base path {!r} for metainfo.yaml'.format(path))
        return None
    if not os.path.isfile(metainfo_file):
        return None

    try:
        metainfo = yaml.safe_load(open(metainfo_file))
    except Exception as e:
        print(e)
        logging.warning(f'Could not load metainfo.yaml for {path}, skipping it')
        return None

    if metainfo is None:
        logging.warning(f'Empty metainfo.yaml for {path}, skipping it')
        return None

    if 'subgroup' in metainfo and 'group' not in metainfo:
        logging.warning(f'Subgroup but no group in {path}, skipping it')
        return None

    # Suppose we get a relative path passed in (e.g. on the command-line,
    # path ".." because we're building the documentation in a subdirectory of a source
    # checkout) then we don't want dirname to be "..", but the name that
    # that resolves to.
    dirname = os.path.basename(os.path.abspath(path))
    if 'fancyname' in metainfo:
        fancyname = metainfo['fancyname']
    else:
        fancyname = utils.parse_fancyname(path)

    if not fancyname:
        logging.warning(f'Could not find fancy name for {path}, skipping it')
        return None
    # A fancyname has 1st char capitalized
    fancyname = fancyname[0].capitalize() + fancyname[1:]

    if 'repo_id' in metainfo:
        repo_id = metainfo['repo_id']
    else:
        repo_id = dirname

    qdoc: bool = False

    if 'qdoc' in metainfo:
        qdoc = metainfo['qdoc']

    metainfo.update({
        'fancyname': fancyname,
        'name': dirname,
        'repo_id': repo_id,
        'public_lib': metainfo.get('public_lib', False),
        'dependency_diagram': None,
        'path': path,
        'qdoc': qdoc,
    })

    # replace legacy platform names
    if 'platforms' in metainfo:
        platforms = metainfo['platforms']
        for index, x in enumerate(platforms):
            if x['name'] == "MacOSX":
                x['name'] = "macOS"
                platforms[index] = x
                logging.warning('{fancyname} uses outdated platform name, please replace "MacOSX" with "macOS".'
                                .format_map(metainfo))
        metainfo.update({'platforms': platforms})
    if 'group_info' in metainfo:
        group_info = metainfo['group_info']
        if 'platforms' in group_info:
            platforms = group_info['platforms']
            for index, x in enumerate(platforms):
                if "MacOSX" in x:
                    x = x.replace("MacOSX", "macOS")
                    platforms[index] = x
                    logging.warning('Group {fancyname} uses outdated platform name, please replace "MacOSX" with "macOS".'
                                    .format_map(group_info))
            group_info.update({'platforms': platforms})

    return metainfo


def parse_tree(rootdir):
    """Recursively call create_metainfo() in subdirs of rootdir

    Args:
        rootdir: (string)  Top level directory containing the libraries.

    Returns:
        A list of metainfo dictionary (see create_metainfo()).

    """
    metalist = []
    for path, dirs, _ in os.walk(rootdir, topdown=True):
        # We don't want to do the recursion in the dotdirs
        dirs[:] = [d for d in dirs if not d[0] == '.']
        metainfo = create_metainfo(path)
        if metainfo is not None:
            # There was a metainfo.yaml, which means it was
            # the top of a checked-out repository. Stop processing,
            # because we do not support having repo B checked out (even
            # as a submodule) inside repo A.
            #
            # There are exceptions: messagelib (KDE PIM) contains
            # multiple subdirectories with their own metainfo.yaml,
            # which are listed as public sources.
            dirs[:] = [d for d in dirs if d in metainfo.get('public_source_dirs', [])]
            if metainfo['public_lib'] or 'group_info' in metainfo:
                metalist.append(metainfo)
            else:
                logging.warning('{name} has no public libraries'.format_map(metainfo))

    return metalist


def sort_metainfo(metalist, all_maintainers):
    """Extract the structure (Product/Subproduct/Library) from the metainfo
    list.

    Args:
        metalist: (list of dict) lists of the metainfo extracted in parse_tree().
        all_maintainers: (dict of dict)  all possible maintainers.

    Returns:
        A list of Products, a list of groups (which are products containing
    several libraries), a list of Libraries and the available platforms.
    """
    products = dict()
    groups = []
    libraries = []
    available_platforms = {'Windows', 'macOS', 'Linux', 'Android', 'FreeBSD'}

    # First extract the structural info
    for metainfo in metalist:
        product = extract_product(metainfo, all_maintainers)
        if product is not None:
            products[product.name] = product

    # Second extract the libraries
    for metainfo in metalist:
        try:
            platforms = metainfo['platforms']
            platform_lst = [x['name'] for x in platforms
                            if x['name'] not in (PLATFORM_ALL,
                                                 PLATFORM_UNKNOWN)]

            available_platforms.update(set(platform_lst))
        except (KeyError, TypeError):
            logging.warning('{fancyname} library lacks valid platform definitions'
                            .format_map(metainfo))
            platforms = [dict(name=PLATFORM_UNKNOWN)]

        dct = dict((x['name'], x.get('note', '')) for x in platforms)

        expand_platform_all(dct, available_platforms)
        platforms = dct

        if metainfo['public_lib']:
            lib = Library(metainfo, products, platforms, all_maintainers)
            libraries.append(lib)

    for key in products.copy():
        if len(products[key].libraries) == 0:
            del products[key]
        elif products[key].part_of_group:
            groups.append(products[key])

    return list(products.values()), groups, libraries, available_platforms


def extract_product(metainfo, all_maintainers):
    """Extract a product from a metainfo dictionary.

    Args:
        metainfo: (dict) metainfo created by the create_metainfo() function.
        all_maintainers: (dict of dict) all possible maintainers

    Returns:
        A Product or None if the metainfo does not describe a product.
    """

    if 'group_info' not in metainfo and 'group' in metainfo:
        # This is not a product but a simple lib
        return None

    try:
        product = Product(metainfo, all_maintainers)
        return product
    except ValueError as e:
        logging.error(e)
        return None
