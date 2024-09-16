#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#workdir=$(cd ../$(dirname $0)/deepin-movie/build-ut; pwd)
executable=deepin-movie-test #可执行程序的文件名

platform=`uname -m`
echo ${platform}

cd ./tests/deepin-movie/

mkdir -p html
mkdir -p report

echo " ===================CREAT LCOV REPROT==================== "
lcov --directory ./CMakeFiles/deepin-movie-test.dir --zerocounters
ASAN_OPTIONS="fast_unwind_on_malloc=1" ./$executable
lcov --directory . --capture --output-file ./html/${executable}_Coverage.info

echo " =================== do filter begin ==================== "
if [ ${platform} = x86_64 ];then
lcov --remove ./html/${executable}_Coverage.info 'tests/CMakeFiles/${executable}.dir/${executable}_autogen/*/*' '${executable}_autogen/*/*/*.cpp' '*/usr/include/*' '*/tests/*' '/usr/local/*' '*/src/common/utility_x11.*' '*/src/common/settings_translation.cpp' '*/src/common/event_monitor.cpp' '*/src/widgets/videoboxbutton.cpp' '*/src/common/hwdec_probe.*'  '*/src/common/platform/*' '*/src/widgets/platform/*'  -o ./html/${executable}_Coverage_fileter.info
else
lcov --remove ./html/${executable}_Coverage.info 'tests/CMakeFiles/${executable}.dir/${executable}_autogen/*/*' '${executable}_autogen/*/*/*.cpp' '*/usr/include/*' '*/tests/*' '/usr/local/*' '*/src/common/utility_x11.*' '*/src/common/settings_translation.cpp' '*/src/common/event_monitor.cpp' '*/src/widgets/videoboxbutton.cpp' '*/src/backends/mpv/mpv_glwidget.cpp' '*/src/common/thumbnail_worker.*'  '*/src/common/platform/*' '*/src/widgets/platform/*' -o ./html/${executable}_Coverage_fileter.info
echo true
fi
echo " =================== do filter end ====================== "
    
genhtml -o ./html ./html/${executable}_Coverage_fileter.info
    
mv ./html/index.html ./html/cov_${executable}.html
mv asan.log* asan_${executable}.log

cp -r ./html/ ../../
cp -r ./report/ ../../
cp ./asan_${executable}.log ../../

#ls report/

exit 0
