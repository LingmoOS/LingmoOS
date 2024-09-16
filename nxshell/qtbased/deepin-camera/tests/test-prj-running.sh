remove_cache(){
    export DISPLAY=":0"
    export QT_QPA_PLATFORM=
    rm -rf ~/Pictures/相机/*.jpg
    rm -rf ~/Pictures/Camera/*.jpg
    rm -rf ~/Videos/相机/*.webm
    rm -rf ~/Videos/Camera/*.webm
    rm -rf ../build-ut
    rm -rf ~/Videos/Camera
    rm -rf ~/Pictures/Camera
}

make_new_dir(){
     mkdir ../build-ut
     mkdir -p ../build-ut/html
     mkdir -p ../build-ut/report
     mkdir ~/Videos/Camera/
     mkdir ~/Pictures/Camera/
}

copy_test_files() {
     cp -r /data/source/deepin-camera/jpegtest/*.jpg ~/Pictures/相机/
     cp -r /data/source/deepin-camera/jpegtest/*.jpg ~/Pictures/Camera/
     cp -r /data/source/deepin-camera/webmtest/*.webm ~/Videos/相机/
     cp -r /data/source/deepin-camera/webmtest/*.webm ~/Videos/Camera/
}

build_exe(){
     cd ../build-ut
     cmake -DCMAKE_BUILD_TYPE=Debug ..
     make -j16
}

create_coverage_report(){
     workdir=$(cd ../$(dirname $0)/build-ut; pwd)
     PROJECT_NAME=deepin-camera-test #可执行程序的文件名
     PROJECT_REALNAME=deepin-camera  #项目名称

     echo " ===================CREAT LCOV REPROT==================== "
     lcov --directory ./tests/CMakeFiles/${PROJECT_REALNAME}.dir --zerocounters
     ASAN_OPTIONS="fast_unwind_on_malloc=1" ./tests/${PROJECT_NAME} --gtest_output=xml:./report/report_deepin-camera.xml
     lcov --directory . --capture --output-file ./html/${PROJECT_REALNAME}_Coverage.info
     echo " =================== do filter begin ==================== "
     lcov --remove ./html/${PROJECT_REALNAME}_Coverage.info 'CMakeFiles/${PROJECT_NAME}.dir/deepin-camera-test_autogen/*/*' '${PROJECT_NAME}_autogen/*/*' 'googletest/*/*' '*/usr/include/*' '*/tests/*' '*/src/src/qtsingleapplication/*' '*/src/src/basepub/printoptionspage.cpp' '*/src/src/dbus_adpator.cpp' '*/src/src/settings_translation.cpp' '/usr/local/*' '*/config.h' '*/src/src/thumbnailsbar.*' '*/src/src/thumbwidget.*' -o ./html/${PROJECT_REALNAME}_Coverage_fileter.info
     echo " =================== do filter end ==================== "

     genhtml -o ./html ./html/${PROJECT_REALNAME}_Coverage_fileter.info
     echo " -- Coverage files have been output to ${CMAKE_BINARY_DIR}/html"
     mv ./html/index.html ./html/cov_${PROJECT_REALNAME}.html
     mv asan.log* asan_${PROJECT_REALNAME}.log
}


remove_cache
make_new_dir
copy_test_files

export DISPLAY=":0"
export QT_QPA_PLATFORM=
echo $DISPLAY
echo $QT_QPA_PLATFORM

build_exe
create_coverage_report
exit 0

