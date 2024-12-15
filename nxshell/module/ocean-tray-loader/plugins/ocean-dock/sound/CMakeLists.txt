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
    "../dbus/types/audioport.h"
    "../dbus/types/audioport.cpp"
    "../dbus/types/audioportlist.h"
    "../dbus/types/audioportlist.cpp"
)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Svg DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget Tools)

pkg_check_modules(XCB_EWMH REQUIRED xcb-ewmh x11 xcursor)

dtk_add_dbus_interface(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/../dbus/xml/org.lingmo.ocean.Audio1.xml
    audio1interface
)

dtk_add_dbus_interface(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/../dbus/xml/org.lingmo.ocean.Audio1.Sink.xml
    audio1sinkinterface
)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${DBUS_INTERFACES} ${SRCS} resources/sound.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ../dbus
    ../../../interfaces
    ../util
    ../common
    ../widgets
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${XCB_EWMH_LIBRARIES}
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    )

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins/system-trays)

dtk_add_config_meta_files(APPID org.lingmo.ocean.dock FILES ../configs/org.lingmo.ocean.dock.plugin.sound.json) # compat
dtk_add_config_meta_files(APPID org.lingmo.ocean.tray-loader FILES ../configs/org.lingmo.ocean.dock.plugin.sound.json)
