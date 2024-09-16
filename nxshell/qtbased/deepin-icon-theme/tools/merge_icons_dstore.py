#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2014 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

import os
import urllib2
import json

def get_all_icons():
    result = {}

    try:
        metadata = urllib2.urlopen("http://api.lastore.deepin.org/metadata")
        metadata_obj = json.loads(metadata.read())
        for package in metadata_obj["data"]:
            package_id = package.get("id", "")
            package_icon = (package.get("icons", []) or [{"src":""}])[0].get("src", "")

            if package_id and package_icon:
                result[package_id] = package_icon
    except Exception, e:
        print e

    return result


def update_record(package_id, package_icon):
    pardir = os.path.dirname
    abspath = os.path.abspath

    dirname = pardir(pardir(abspath(__file__)))
    file_path = os.path.join(dirname, "deepin/apps/48/", "%s.svg") % package_id
    try:
        data = urllib2.urlopen(package_icon)
        with open(file_path, "w") as icon:
            print "writing data to %s..." % file_path
            icon.write(data.read())
    except Exception, e:
        print e

if __name__ == "__main__":
    icons = get_all_icons()
    for package_id, package_icon in icons.items():
        if (package_id.encode("utf-8").startswith("apps")) and \
            package_icon.encode("utf-8").endswith("svg"):
            update_record(package_id, package_icon)
        else:
            print "skip apps that are not maintained by deepin: %s" % package_id