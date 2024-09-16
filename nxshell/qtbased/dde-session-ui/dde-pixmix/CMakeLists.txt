# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Pixmix_SRCS
    "src/*.h"
    "src/*.cpp"
    )
list(REMOVE_ITEM Pixmix_SRCS "${CMAKE_SOURCE_DIR}/dde-pixmix/srcmain.cpp")

set(Pixmix_Includes
    ${DtkWidget_INCLUDE_DIRS}
    Qt5::Widgets
    )
set(Pixmix_Libraries
    ${DtkWidget_LIBRARIES}
    Qt5::Widgets
    )

add_subdirectory("src")
#add_subdirectory("tests")
