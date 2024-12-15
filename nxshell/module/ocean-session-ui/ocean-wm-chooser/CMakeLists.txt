# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Wm_Chooser_SRCS
    "src/*.h"
    "src/*.cpp"
    )

list(REMOVE_ITEM Wm_Chooser_SRCS "${CMAKE_SOURCE_DIR}/ocean-wm-chooser/src/main.cpp")

set(Wm_Chooser_Includes
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
)
set(Wm_Chooser_Libraries
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
)

add_subdirectory("src")
add_subdirectory("tests")
