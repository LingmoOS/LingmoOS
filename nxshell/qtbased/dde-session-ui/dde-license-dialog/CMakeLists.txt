# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE License_Dialog_SRCS
    "src/*.h"
    "src/*.cpp"
    )

list(REMOVE_ITEM License_Dialog_SRCS "${CMAKE_SOURCE_DIR}/dde-license-dialog/src/main.cpp")

set(License_Dialog_Includes
    ${DtkWidget_INCLUDE_DIRS}
    Qt5::Widgets
    Qt5::Concurrent
    )

set(License_Dialog_Libraries
    ${DtkWidget_LIBRARIES}
    Qt5::Widgets
    Qt5::Concurrent
    )

add_subdirectory("src")
add_subdirectory("tests")
