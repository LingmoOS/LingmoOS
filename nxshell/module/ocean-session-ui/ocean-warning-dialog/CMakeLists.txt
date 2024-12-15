# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Warning_Dialog_SRCS
    "src/*.h"
    "src/*.cpp"
)

list(REMOVE_ITEM Warning_Dialog_SRCS "${CMAKE_SOURCE_DIR}/ocean-warning-dialog/src/main.cpp")

set(Warning_Dialog_Includes
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)
set(Warning_Dialog_Libraries
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)

add_subdirectory("src")
add_subdirectory("tests")
