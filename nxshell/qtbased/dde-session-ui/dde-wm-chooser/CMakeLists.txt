# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Wm_Chooser_SRCS
    "src/*.h"
    "src/*.cpp"
    )

list(REMOVE_ITEM Wm_Chooser_SRCS "${CMAKE_SOURCE_DIR}/dde-wm-chooser/src/main.cpp")

set(Wm_Chooser_Includes
    ${DtkWidget_INCLUDE_DIRS}
    ${GSETTINGS_INCLUDE_DIRS}
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
    )
set(Wm_Chooser_Libraries
    ${DtkWidget_LIBRARIES}
    ${GSETTINGS_LIBRARIES}
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
    )

add_subdirectory("src")
add_subdirectory("tests")
