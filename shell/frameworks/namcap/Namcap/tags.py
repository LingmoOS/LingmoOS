# Copyright (C) 2003-2023 Namcap contributors, see AUTHORS for details.
# SPDX-License-Identifier: GPL-2.0-or-later

import os

tags: dict[str, str] = {}

DEFAULT_TAGS = "/usr/share/namcap/namcap-tags"


def load_tags(filename=None, machine=False):
    "Loads tags from the given filename"
    global tags
    tags = {}
    if filename is None:
        filename = DEFAULT_TAGS

    f = open(filename)
    for line in f:
        line = line.strip()
        if line.startswith("#") or line == "":
            continue
        machinetag, humantag = line.split("::")
        machinetag = machinetag.strip()
        humantag = humantag.strip()

        if machine:
            tags[machinetag] = machinetag
        else:
            tags[machinetag] = humantag


def format_message(msg):
    """
    Formats a tuple (tag, data)
    """
    tag, data = msg
    return tags[tag] % data


# Try to load tags by default
if os.path.exists(DEFAULT_TAGS):
    load_tags(DEFAULT_TAGS)
elif os.path.exists("namcap-tags"):
    load_tags("namcap-tags")
