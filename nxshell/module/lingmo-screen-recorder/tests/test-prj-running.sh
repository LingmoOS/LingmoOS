#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

rm -rf ../build-ut
mkdir ../build-ut
mkdir ../build-ut/report
mkdir ../build-ut/html

cppcheck --enable=all --output-file=../build-ut/cppcheck_lingmo-screen-recorder.xml --xml  ../src/
cppcheck --enable=all --output-file=../build-ut/cppcheck_ocean-dock-plugins.xml --xml  ../ocean-dock-plugins/

./ut_ocean_dock_plugins/build_run_ut.sh
./ut_screen_shot_recorder/build_run_ut.sh

exit 0