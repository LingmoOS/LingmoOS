#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

executable=deepin-movie-platform-test #可执行程序的文件名

platform=`uname -m`
echo ${platform}

cd ./tests/deepin-movie-platform/

mkdir -p html
mkdir -p report
rm -fr /data/source/deepin-movie-reborn/movie/play.conf
cp ../../../tests/deepin-movie-platform/play.conf /data/source/deepin-movie-reborn/movie/play.conf
echo " ===================CREAT LCOV REPROT==================== "
lcov --directory ./CMakeFiles/deepin-movie-platform-test.dir --zerocounters
ASAN_OPTIONS="fast_unwind_on_malloc=1" ./$executable
lcov --directory . --capture --output-file ./html/${executable}_Coverage.info
rm -fr /data/source/deepin-movie-reborn/movie/play.conf

echo " =================== do filter begin ==================== "
if [ ${platform} = x86_64 ];then 
lcov --remove ./html/${executable}_Coverage.info 'tests/CMakeFiles/${executable}.dir/${executable}_autogen/*/*' '${executable}_autogen/*/*/*.cpp' '*/usr/include/*' '*/tests/*' '/usr/local/*' '*/src/common/utility_x11.*' '*/src/common/settings_translation.cpp' '*/src/common/event_monitor.cpp' '*/src/widgets/videoboxbutton.cpp' '*/src/common/hwdec_probe.*' '*/src/backends/mpv/mpv_glwidget.cpp' '*/src/common/thumbnail_worker.*' '*/src/common/mainwindow.*' '*/src/common/dbus_adpator.*' '*/src/common/dmr_setting.*' '*/src/widgets/animationlabel.*' '*/src/widgets/movie_progress_indicator.*' '*/src/widgets/notification_widget.*' '*/src/widgets/playlist_widget.*' '*/src/widgets/toolbox_proxy.*' '*/src/widgets/volumeslider.*' -o ./html/${executable}_Coverage_fileter.info
else
lcov --remove ./html/${executable}_Coverage.info 'tests/CMakeFiles/${executable}.dir/${executable}_autogen/*/*' '${executable}_autogen/*/*/*.cpp' '*/usr/include/*' '*/tests/*' '/usr/local/*' '*/src/common/utility_x11.*' '*/src/common/settings_translation.cpp' '*/src/common/event_monitor.cpp' '*/src/widgets/videoboxbutton.cpp' '*/src/backends/mpv/mpv_glwidget.cpp' '*/src/common/platform/thumbnail_worker.*'  '*/src/common/thumbnail_worker.*' '*/src/common/mainwindow.*' '*/src/common/dbus_adpator.*' '*/src/common/dmr_setting.*' '*/src/widgets/animationlabel.*' '*/src/widgets/movie_progress_indicator.*' '*/src/widgets/notification_widget.*' '*/src/widgets/playlist_widget.*' '*/src/widgets/toolbox_proxy.*' '*/src/widgets/volumeslider.*'  -o ./html/${executable}_Coverage_fileter.info
echo true
fi

echo " =================== do filter end ====================== "
    
genhtml -o ./html ./html/${executable}_Coverage_fileter.info
    
mv ./html/index.html ./html/cov_${executable}.html
mv asan.log* asan_${executable}.log

cp -r ./html/ ../../
cp -r ./report/ ../../
cp ./asan_${executable}.log ../../

//ls report/

exit 0
