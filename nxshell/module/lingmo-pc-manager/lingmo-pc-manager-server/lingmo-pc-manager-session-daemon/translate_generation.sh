#!/bin/bash

# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at lingmoos.com

ts_list=($(ls translations/*.ts))

for ts in "${ts_list[@]}"; do
	printf "\nprocess ${ts}\n"
	lrelease "${ts}"
done
