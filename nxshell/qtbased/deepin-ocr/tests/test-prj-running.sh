#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

#export QT_QPA_PLATFORM='offscreen'
export QTEST_FUNCTION_TIMEOUT='800000'
export DISPLAY=":0"
export QT_QPA_PLATFORM=

export QT_LOGGING_RULES="qt.qpa.xcb.xcberror=false"
export QT_LOGGING_RULES="qt.qpa.xcb.*=false"
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_LOGGING_RULES="*=false"

rm -rf ${HOME}/Pictures/ocr_test
mkdir -p ${HOME}/Pictures/ocr_test
cp -r ./testResource ${HOME}/Pictures/ocr_test
cp ../assets/model/* ${HOME}/Pictures/ocr_test/*

cd ..
rm -rf ./build-ut
rm -rf ./build
mkdir build-ut

cmake . -B build -D DOTEST=ON
cd build

#自动读取当前处理器核心数，但考虑到服务器上会同时存在多个构建，完全占用服务器CPU会导致构建变慢，所以限制使用的核心不超过8个
JOBS=`cat /proc/cpuinfo| grep "processor"|  wc -l`
if [ $JOBS -gt 8 ]
then JOBS=8
elif [ $JOBS -eq 0 ]
then JOBS=1
fi

echo use processor count: $JOBS
make deepin-ocr_test -j$JOBS

lcov --directory ./src/CMakeFiles/deepin-ocr_test.dir --zerocounters
./src/deepin-ocr_test

lcov --directory . --capture --output-file ./coverageResult/deepin-ocr_Coverage.info
echo \ ===================\ do\ filter\ begin\ ====================\ 
lcov --remove ./coverageResult/deepin-ocr_Coverage.info '*/3rdparty/*' '*/deepin-ocr_test_autogen/*' '*/deepin-ocr_autogen/*' '*/usr/include/*' '*/usr/local/*' '*/tests/*' '*/googletest/*' -o ./coverageResult/deepin-ocr_Coverage.info
echo \ ===================\ do\ filter\ end\ ====================\ 
genhtml -o ./coverageResult/report ./coverageResult/deepin-ocr_Coverage.info

sleep 2

lcov --directory . --capture --output-file ./coverageResult/deepin-ocr_Coverage.info
echo \ ===================\ do\ filter\ begin\ ====================\ 
lcov --remove ./coverageResult/deepin-ocr_Coverage.info '*/3rdparty/*' '*/deepin-ocr_test_autogen/*' '*/deepin-ocr_autogen/*' '*/usr/include/*' '*/usr/local/*' '*/tests/*' '*/googletest/*' -o ./coverageResult/deepin-ocr_Coverage.info
echo \ ===================\ do\ filter\ end\ ====================\ 
genhtml -o ./coverageResult/report ./coverageResult/deepin-ocr_Coverage.info



cd ./../build-ut

cp -r ./../build/coverageResult/report/ ./
mv report html
cd html
mv index.html cov_deepin-ocr.html

cd ..
mkdir report
cd report
cp ./../../build/report/report_deepin-ocr.xml ./

cd ..
cp ./../build/asan_deepin-ocr.log* ./asan_deepin-ocr.log

exit 0
