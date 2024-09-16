#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

rm -rf ${HOME}/.config/deepin-movie-test
rm -rf ${HOME}/Pictures/DMovie

rm -rf ../$(dirname $0)/build-ut
#rm -rf ../$(dirname $0)/test-deepin-movie
#rm -rf ../$(dirname $0)/ut_dmr-test
#mkdir ../$(dirname $0)/test-deepin-movie
#mkdir ../$(dirname $0)/test-deepin-movie/build-ut
#mkdir ../$(dirname $0)/ut_dmr-test
#mkdir ../$(dirname $0)/ut_dmr-test/build-ut
mkdir ../$(dirname $0)/build-ut

cd ../build-ut

export DISPLAY=":0"
export QT_QPA_PLATFORM=
echo $QT_QPA_PLATFORM
export QTEST_FUNCTION_TIMEOUT='500000'
#export QT_SELECT=qt5
cmake -DCMAKE_BUILD_TYPE=Debug ../
make -j16


#../tests/deepin-movie-platform/ut-build-run.sh
../tests/deepin-movie/ut-build-run.sh
../tests/ut_dmr-test/ut-build-run.sh

exit 0
