# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Global_Util_SRCS
    multiscreenmanager.cpp
    multiscreenmanager.h
    public_func.cpp
    public_func.h
    util_updateui.cpp
    util_updateui.h
)

add_library(session-ui-util-shared OBJECT
    ${Global_Util_SRCS}
)

target_include_directories(session-ui-util-shared
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(session-ui-util-shared
PRIVATE
    Qt5::Widgets
    Dtk::Core
)
