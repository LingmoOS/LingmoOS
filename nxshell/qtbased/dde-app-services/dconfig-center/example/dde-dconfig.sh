#!/bin/bash

# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-only

# configure file is in the position. e.g: /usr/share/dsg/configs/dconfig-example/example.json
# global cache is in the position. e.g: /var/dsg/appdata/configs/dconfig-example/configs/example.json
# user cache is in the position. e.g: /home/userhome/.config/dsg/configs/dconfig-example/example.json

# list all appid.
dde-dconfig list

# list all resource for the appid.
dde-dconfig list dconfig-example

# list all matched common resource.
dde-dconfig list -r=""

#list all subpath for the resource.
dde-dconfig list dconfig-example -r=example


# query all method for `-m` argument.
dde-dconfig get

# query all keys for the resource, which `subpath` is '/a'.
dde-dconfig get dconfig-example -r=example -s=/a

# query name for the key, which `language` is 'zh_CN'.
dde-dconfig get dconfig-example -r=example -m=name canExit -l=zh_CN

# query description for the key.
dde-dconfig get dconfig-example -r=example -m=description canExit

# query visibility for the key.
dde-dconfig get dconfig-example -r=example -m=visibility canExit

# query permissions for the key.
dde-dconfig get dconfig-example -r=example -m=permissions canExit

# query isDefaultValue for the key.
dde-dconfig get dconfig-example -r=example -m=isDefaultValue canExit

# set value for the key.
dde-dconfig set dconfig-example -r=example canExit -v=false
# set value for the key, which data type is array and value is json format.
dde-dconfig set dconfig-example -r=example array -v="[ \"value1\", \"value2\" ]"
# set value for the key, which data type is map and value is json format.
dde-dconfig set dconfig-example -r=example map -v="{ \"key1\": \"value1\", \"key2\": \"value2\" }"

# reset value for the key.
dde-dconfig reset dconfig-example -r=example canExit

# query value for the key
dde-dconfig get dconfig-example -r=example canExit
# query value for the key, which data type is array and value is json format.
dde-dconfig get dconfig-example -r=example array

# query value for the key canExit of user(uid)
dde-dconfig get dconfig-example -r=example -k canExit -u $(id -u)
