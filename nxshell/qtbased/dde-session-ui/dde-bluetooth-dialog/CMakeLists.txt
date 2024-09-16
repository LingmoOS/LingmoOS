# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Bluetooth_Dialog_SRCS
    "src/*.h"
    "src/*.cpp"
)

list(REMOVE_ITEM Bluetooth_Dialog_SRCS "${CMAKE_SOURCE_DIR}/dde-bluetooth-dialog/src/main.cpp")

set(Bluetooth_Dialog_Includes
    ${DtkWidget_INCLUDE_DIRS}
    ${GSETTINGS_INCLUDE_DIRS}
    Qt5::Widgets
    Qt5::DBus
)
set(Bluetooth_Dialog_Libraries
    ${DtkWidget_LIBRARIES}
    ${GSETTINGS_LIBRARIES}
    Qt5::Widgets
    Qt5::DBus
)

add_subdirectory("src")
add_subdirectory("tests")
