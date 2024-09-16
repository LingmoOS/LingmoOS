# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "sound")

project(${PLUGIN_NAME})

# Sources files
file(GLOB_RECURSE SRCS
    "*.h"
    "*.cpp"
    "../widgets/*.h"
    "../widgets/*.cpp"
    "../util/imageutil.h"
    "../util/imageutil.cpp"
    "../util/horizontalseparator.h"
    "../util/horizontalseparator.cpp"
    "../common/*.h"
    "../common/*.cpp"
)

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)

pkg_check_modules(XCB_EWMH REQUIRED xcb-ewmh x11 xcursor)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} resources/sound.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
    ../../../interfaces
    ../util
    ../tray
    ../common
    ../widgets
    )

target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    )

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/system-trays)

dconfig_meta_files(APPID org.deepin.dde.dock FILES ../configs/org.deepin.dde.dock.plugin.sound.json) # compat
dconfig_meta_files(APPID org.deepin.dde.tray-loader FILES ../configs/org.deepin.dde.dock.plugin.sound.json)
