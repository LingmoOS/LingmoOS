# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "keyboard-layout")

project(${PLUGIN_NAME})

# Sources files
file(GLOB_RECURSE SRCS "*.h" "*.cpp"
    "../dbus/types/keyboardlayoutlist.h"
    "../dbus/types/keyboardlayoutlist.cpp"
)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Svg DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget Tools)

set_source_files_properties(
    org.fcitx.Fcitx.xml
    PROPERTIES INCLUDE fcitxinputmethoditem.h
    CLASSNAME FcitxInputMethodProxy
)

qt_add_dbus_interfaces(DBUS_INTERFACES
    org.fcitx.Fcitx.xml
)

dtk_add_dbus_interface(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/../dbus/xml/org.lingmo.ocean.InputDevice1.Keyboard.xml
    inputdevice1keyboardinterface
)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} ${DBUS_INTERFACES})
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ../dbus
    ../../../interfaces
    ../common
    ../util
)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins/system-trays/)
install(FILES ./keybord_layout.json DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/ocean-dock/indicator)
