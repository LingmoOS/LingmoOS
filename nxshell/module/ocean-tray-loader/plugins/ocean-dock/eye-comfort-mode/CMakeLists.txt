# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "eye-comfort-mode")

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
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Svg DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget Tools)

dtk_add_dbus_interface(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/../dbus/xml/org.lingmo.ocean.Appearance1.xml
    appearance1interface
)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${DBUS_INTERFACES} ${SRCS} resources/eye-comfort-mode.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ../../../interfaces
    ../widgets
    ../common
    ../util
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins)
install(FILES "resources/oceanui-eye-comfort-mode.dci" DESTINATION share/ocean-dock/icons/oceanui-setting)
