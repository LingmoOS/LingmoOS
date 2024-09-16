
# 设置工程名字
project(deepin-camera-test VERSION 1.0.0)
set(CMD_NAME deepin-camera-test)
set(TARGET_NAME deepin-camera-test)
# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.0)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
option (UNITTEST "Use UNITTEST" ON)
add_definitions(-DUNITTEST)

execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)



include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_INCLUDE
    ../libcam/libcam_v4l2core
    ../libcam/libcam_render
    ../libcam/libcam_encoder
    ../libcam/libcam_audio
    ../libcam/libcam
    ../src/src/basepub
    ../src/src/accessibility
    ../src/src/accessible
    ../src/src/qtsingleapplication
    ../src/
    ./stub
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INSTALL_PREFIX}/include/libusb-1.0
    ${CMAKE_INSTALL_PREFIX}/include/SDL2
    ${PROJECT_SOURCE_DIR}/../deepin-camera
    ${PROJECT_SOURCE_DIR}/googletest/include
    )
# 设置Qt模块
set(QtModule Core Gui Widgets DBus Concurrent LinguistTools Multimedia PrintSupport Svg)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})
find_package(PkgConfig REQUIRED)

file (GLOB_RECURSE CXXSOURCES
    ./*.cpp
    ../src/src/basepub/*.cpp
    ../src/src/*.cpp
    ../src/src/qtsingleapplication/*.cpp
    ./stub/*.cpp
    )

file (GLOB_RECURSE CSOURCES
    ../src/src/*.c
    ../libcam/*.c
    ../src/src/basepub/*.c
    )

file (GLOB_RECURSE HEADERS
    ../src/src/basepub/*.h
    ../libcam/*.h
    ../src/src/qtsingleapplication/*.h
    ../src/src/*.h
    ../src/src/accessibility/*.h
    ../src/src/accessible/*.h
    ../tests/*.h
    ./stub/*.h
    ./stub/*.hpp
    )
file (GLOB_RECURSE RESOURCES  ../src/assets/assets.qrc)

configure_file(../src/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

add_executable( ${TARGET_NAME} ${CXXSOURCES} ${CSOURCES} ${RESOURCES}
    )


pkg_check_modules(3rd_lib REQUIRED
        dtkgui
        dtkwidget gstreamer-1.0 gstreamer-app-1.0
        pciaccess
#        libv4l2
#        libudev
#        libusb-1.0
#        libavcodec
#        libavutil
#        libavformat
#        libswscale
#        libpng
#        sdl2
#        libpulse
#        libswresample
#        libffmpegthumbnailer
        )
# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)

target_include_directories(${TARGET_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${PROJECT_INCLUDE} ${GLOB_RECURSE})
# 将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES}
    gtest
    pthread
    Qt5Test
    dl
#    portaudio
#    asound
#    z

    imagevisualresult
    )

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

#configure_file(${PROJECT_SOURCE_DIR}/config.h.in  @ONLY)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 添加gocv覆盖率文件的输出
#set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
set(CMAKE_Ｌ_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/objects)

#add_custom_target(test
#    COMMAND mkdir -p report
#    COMMAND mkdir -p html
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

#add_custom_command(TARGET test
#    COMMAND echo " ===================CREAT LCOV REPROT==================== "
#    COMMAND lcov --directory ./tests/CMakeFiles/deepin-camera-test.dir --zerocounters
#    COMMAND ./tests/${PROJECT_NAME} --gtest_output=xml:./report/report.xml
#    COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info

#    COMMAND echo " =================== do filter begin ==================== "
#    COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info 'CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_autogen/*/*'
#    '${PROJECT_NAME}_autogen/*/*'
#    'googletest/*/*'
#    '*${CMAKE_INSTALL_PREFIX}/include/*'
#    '*/tests/*'
#    '*/src/qtsingleapplication/*'
#    '*/basepub/printoptionspage.cpp'
#    '*/src/src/dbus_adpator.cpp'
#    '*/src/src/settings_translation.cpp'
#    '${CMAKE_INSTALL_PREFIX}/local/*'
#    -o ./coverageResult/${PROJECT_NAME}_Coverage_fileter.info
#    COMMAND echo " =================== do filter end ==================== "

#    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage_fileter.info
#    COMMAND echo " -- Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    )
#add_dependencies(test ${PROJECT_NAME})
