# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Lowpower_SRCS
    "src/*.h"
    "src/*.cpp"
    )

list(REMOVE_ITEM Lowpower_SRCS "${CMAKE_SOURCE_DIR}/dde-lowpower/src/main.cpp")

set(Lowpower_Includes
    ${DtkWidget_INCLUDE_DIRS}
    Qt5::Widgets
    Qt5::DBus
    )

set(Lowpower_Libraries
    ${DtkWidget_LIBRARIES}
    Qt5::Widgets
    Qt5::DBus
    )

add_subdirectory("src")
add_subdirectory("tests")
