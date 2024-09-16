# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

option(ENABLE_DEMO "Build demo" OFF)

if (ENABLE_DEMO)
    add_subdirectory("plugin-qt")
    add_subdirectory("plugin-sdbus")

    add_subdirectory("libqdbusservice-demo")
endif()

# add_subdirectory("hook-demo")
