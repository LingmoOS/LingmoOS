# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(X11 REQUIRED)

file(GLOB_RECURSE Touchscreen_Dialog_SRCS
    "src/*.h"
    "src/*.cpp"
)

list(REMOVE_ITEM Touchscreen_Dialog_SRCS "${CMAKE_SOURCE_DIR}/ocean-touchscreen-dialog/src/main.cpp")

set(Touchscreen_Dialog_Includes
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)
set(Touchscreen_Dialog_Libraries
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${X11_LIBRARIES}
    -lXext
)

add_subdirectory("src")
add_subdirectory("tests")
