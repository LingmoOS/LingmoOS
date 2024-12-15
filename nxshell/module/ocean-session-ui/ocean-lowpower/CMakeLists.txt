# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Lowpower_SRCS
    "src/*.h"
    "src/*.cpp"
)

list(REMOVE_ITEM Lowpower_SRCS "${CMAKE_SOURCE_DIR}/ocean-lowpower/src/main.cpp")

set(Lowpower_Includes
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)

set(Lowpower_Libraries
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)

add_subdirectory("src")
add_subdirectory("tests")
