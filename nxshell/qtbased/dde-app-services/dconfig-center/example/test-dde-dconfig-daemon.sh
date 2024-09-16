#!/bin/bash

# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-only

# configure file is in the position. e.g: /usr/share/dsg/configs/dconfig-example/example.json
# global cache is in the position. e.g: /var/dsg/appdata/configs/dconfig-example/configs/example.json
# user cache is in the position. e.g: /home/userhome/.config/dsg/configs/dconfig-example/example.json

metaLocation=/usr/share/dsg/configs/dconfig-example/example.json


# add configuration file
sudo cp ./configs/example-old.json /usr/share/dsg/configs/dconfig-example/example.json

# create connection
dde-dconfig --get -a=dconfig-example -r=example -k=removedItem

# modify configuration item
dde-dconfig --set -adconfig-example -r=example -k=removedItem -v="new value of removeItem"

# test setValue
dde-dconfig --get -a=dconfig-example -r=example -k=removedItem

# modify configuration file
sudo cp ./configs/example-new.json /usr/share/dsg/configs/dconfig-example/example.json

# update configuration file
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.update string:/usr/share/dsg/configs/dconfig-example/example.json


# get new item
dde-dconfig --get -a=dconfig-example -r=example -k=addItem

# try to get old item
dde-dconfig --get -a=dconfig-example -r=example -k=removedItem


dde-dconfig --set -adconfig-example -r=example -k=addItem -v="new value of addItem"

dde-dconfig --get -a=dconfig-example -r=example -k=addItem

# sync the resource
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.sync string:/usr/share/dsg/configs/dconfig-example/example.json

more ~/.config/dsg/configs/dconfig-example/example.json

# restore scene

rm ~/.config/dsg/configs/dconfig-example/example.json
