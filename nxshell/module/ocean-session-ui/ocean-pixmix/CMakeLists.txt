# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(GLOB_RECURSE Pixmix_SRCS
    "src/*.h"
    "src/*.cpp"
)
list(REMOVE_ITEM Pixmix_SRCS "${CMAKE_SOURCE_DIR}/ocean-pixmix/srcmain.cpp")

set(Pixmix_Includes
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
)
set(Pixmix_Libraries
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_subdirectory("src")
#add_subdirectory("tests")
