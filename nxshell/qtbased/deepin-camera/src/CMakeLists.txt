# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.0)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
project(deepin-camera VERSION 1.0.0)
set(CMD_NAME deepin-camera)
set(TARGET_NAME deepin-camera)
find_package(DWayland)
find_package(KF5Wayland)
if (NOT DWayland_FOUND)
    if (${KF5Wayland_VERSION} VERSION_GREATER 5.54.1)
        option(USE_DEEPIN_WAYLAND "option for wayland support" ON)
    endif()
else()
    option(USE_DEEPIN_WAYLAND "option for wayland support" ON)
endif()
#option (__mips__ "Use UNITTEST" ON)

# cube文件目录，libvisualresult在打包时，会自动将cube文件安装到share目录下
set(LUT_DIR "${CMAKE_INSTALL_PREFIX}/share/libimagevisualresult/filter_cube")
add_definitions(-DLUT_DIR="${LUT_DIR}")

# 判断系统环境
#file(STRINGS "/etc/os-version" MAJOR_STR REGEX "MajorVersion")
execute_process(COMMAND cat /etc/os-version OUTPUT_VARIABLE OS_INFO_STR)
string(REGEX MATCHALL "MajorVersion=[0-9]+" MAJOR_STR "${OS_INFO_STR}")
string(REGEX MATCH "[0-9]+" MAJOR_VERSION "${MAJOR_STR}")
if (MAJOR_VERSION MATCHES "23")
    message("--------------------- OS_BUILD_V23 on")
    add_definitions(-DOS_BUILD_V23)
elseif(MAJOR_VERSION MATCHES "25")
    message("--------------------- OS_BUILD_V25 on")
    option(USE_DEEPIN_V25 "option for v25 support" ON)
    add_definitions(-DOS_BUILD_V25)
endif()

include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (USE_DEEPIN_WAYLAND)
    if(!USE_DEEPIN_V25)
        add_definitions("-DUSE_DEEPIN_WAYLAND")
    endif()
endif()

if(DEFINED LINGLONG_BUILD_ON)
    # 玲珑构建编译开关
    add_definitions(-DLINGLONG_BUILD)
    include_directories(/runtime/include)
    message(STATUS "LINGLONG_BUILD is ON")
else()
    message(STATUS "LINGLONG_BUILD is OFF")
endif()

# 加速编译优化参数
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-needed -fPIE")
    SET(CMAKE_C_FLAGS "$ENV{CFLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-needed -fPIE")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pie")
else()
    SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -g -Wl,--as-needed -fPIE")
    SET(CMAKE_C_FLAGS "$ENV{CFLAGS} -O3  -Wl,--as-needed -fPIE")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pie")
endif()

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

set(PROJECT_INCLUDE
    ../
    ./src/
    ../libcam/libcam_v4l2core
    ../libcam/libcam_render
    ../libcam/libcam_encoder
    ../libcam/libcam_audio
    ../libcam/libcam
    ./src/basepub
    ./src/qtsingleapplication
    ./src/accessibility
    ./src/accessible
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INSTALL_PREFIX}/include/libusb-1.0
    )
    
# 设置Qt模块
set(QtModule Core Gui Widgets DBus Concurrent LinguistTools Multimedia PrintSupport Svg)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})
find_package(PkgConfig REQUIRED)

include(FindPkgConfig)
if (USE_DEEPIN_WAYLAND)
    if (DWayland_FOUND)
        message("DWayland founded.")
        add_definitions(-DDWAYLAND)
    elseif(KF5Wayland_FOUND)
        message("KF5Wayland founded.")
        add_definitions(-DKF5WAYLAND)
    endif()
endif()

file (GLOB_RECURSE CXXSOURCES
    ./src/*.cpp
    ./*.cpp
    )

file (GLOB_RECURSE CSOURCES
    ./src/*.c
    ../libcam/*.c
    ./src/basepub/*.c
    )

file (GLOB_RECURSE HEADERS
    ./src/*.h
    ../libcam/*.h
    ./src/basepub/*.h
    ./src/qtsingleapplication/*.h
    ./src/accessibility/*.h
    ./src/accessible/*.h
    )

file (GLOB_RECURSE RESOURCES  ${PROJECT_SOURCE_DIR}/assets/assets.qrc)

pkg_check_modules(3rd_lib REQUIRED
        dtkwidget dtkcore gobject-2.0
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
        )
list(APPEND PROJECT_INCLUDE ${CMAKE_INSTALL_PREFIX}/include/glib-2.0 ${CMAKE_INSTALL_PREFIX}/include/gstreamer-1.0)

# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)

file (GLOB TS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}*.ts)
#message(STATUS "qm")
#message(STATUS ${PROJECT_SOURCE_DIR})

#file(GLOB files LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/translations/${CMD_NAME}*.ts)
#foreach(file IN LISTS files)
#    message(STATUS ${cur})
#endforeach(file)

set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${CXXSOURCES} ${TS})

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(QM ${CXXSOURCES} ${TS})
endif ()

# 设置不删除生成的文件夹内容文件
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

if (USE_DEEPIN_WAYLAND)
    if (DWayland_FOUND)
        set(LIBS DWaylandClient DWaylandServer)
    else()
        message("KF5 environment")
        set(LIBS KF5::WaylandClient KF5::WaylandServer)
    endif()
endif()

add_executable( ${TARGET_NAME} ${CXXSOURCES} ${CSOURCES} ${RESOURCES} ${QM} )

target_include_directories(${TARGET_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${PROJECT_INCLUDE} ${GLOB_RECURSE})
# 将第三方库链接进来(注意 *_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${3rd_lib_LIBRARIES}
    pthread
    dl
    imagevisualresult
    ${LIBS}
    )

include_directories("${CMAKE_INSTALL_PREFIX}/include")

# 将工程与Qt模块链接起来
qt5_use_modules(${TARGET_NAME} ${QtModule})

#application
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

#desk desktop
install(FILES ${PROJECT_SOURCE_DIR}/deepin-camera.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

#table desktop
install(FILES ${PROJECT_SOURCE_DIR}/table/deepin-camera.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/due-shell/shortcuts)

#icons
install(FILES ${PROJECT_SOURCE_DIR}/assets/deepin-camera.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

#table_icons
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/panel/Icons DESTINATION ${CMAKE_INSTALL_DATADIR}/due-shell/shortcuts)

#manual
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/deepin-camera DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)

#debus_service
install(FILES ${PROJECT_SOURCE_DIR}/com.deepin.Camera.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

# Install camera config file.
install(FILES assets/camera.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-camera)

##translations
#install(FILES ${PROJECT_SOURCE_DIR}/translations/*.qm DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_SOURCE_DIR}/translations)

#translations
set(APPSHAREDIR  ${CMAKE_INSTALL_DATADIR}/${TARGET_NAME})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")

#hw机型增加DConfig配置
set(APPID org.deepin.camera)
set(configFile ${PROJECT_SOURCE_DIR}/assets/org.deepin.camera.encode.json)
if (DEFINED DSG_DATA_DIR)
    message("-- DConfig is supported by DTK")
    dconfig_meta_files(APPID ${APPID} FILES ${configFile})
else()
    install(FILES ${configFile} DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/org.deepin.camera/)
endif()
