#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

workspace=$1

cd $workspace

dpkg-buildpackage -b -d -uc -us

project_path=$(cd `dirname $0`; pwd)
#获取工程名
project_name="${project_path##*/}"
echo $project_name

#获取打包生成文件夹路径
pathname=$(find . -name obj*)

echo $pathname

cd $pathname

mkdir -p coverage

lcov -d ../ --capture -o ./coverage/coverage.info

lcov --extract ./coverage/coverage.info '*/src/*' -o ./coverage/coverage.info

lcov --remove ./coverage/coverage.info '*/tests/*' '*/3rdparty/*' -o  ./coverage/coverage.info

mkdir ../report
genhtml -o ../report ./coverage/coverage.info

exit 0
