# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(Widgets_SRCS
    fullscreenbackground.cpp
    fullscreenbackground.h
    propertygroup.cpp
    propertygroup.h
)

add_library(session-ui-widgets-shared OBJECT
    ${Widgets_SRCS}
)

target_include_directories(session-ui-widgets-shared
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(session-ui-widgets-shared
PRIVATE
    Qt5::Widgets
)
