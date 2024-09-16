# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: MIT

cd ./src/plugin-sdbus/plugin-oom
sh lupdate.sh
cd ../../../

tx push -s --branch m20
