# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "dnd-mode")

project(${PLUGIN_NAME})

# Sources files
file(GLOB_RECURSE SRCS
    "*.h"
    "*.cpp"
    "../widgets/*.h"
    "../widgets/*.cpp"
    "../common/*.h"
    "../common/*.cpp"
)

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)

pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} resources/dnd_mode.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ../../../interfaces
    ../widgets
    ../util
    ../common
)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
install(FILES "resources/dnd-mode.svg" DESTINATION share/dde-dock/icons/dcc-setting)
