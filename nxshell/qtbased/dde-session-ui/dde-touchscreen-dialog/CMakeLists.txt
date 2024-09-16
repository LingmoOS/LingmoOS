# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Touchscreen_Dialog_SRCS
    "src/*.h"
    "src/*.cpp"
    )

list(REMOVE_ITEM Touchscreen_Dialog_SRCS "${CMAKE_SOURCE_DIR}/dde-touchscreen-dialog/src/main.cpp")

set(Touchscreen_Dialog_Includes
    ${DtkWidget_INCLUDE_DIRS}
    Qt5::X11Extras
    Qt5::Widgets
    Qt5::DBus
    )
set(Touchscreen_Dialog_Libraries
    ${DtkWidget_LIBRARIES}
    Qt5::X11Extras
    Qt5::Widgets
    Qt5::DBus
    -lXext
    )

add_subdirectory("src")
add_subdirectory("tests")
