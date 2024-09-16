# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dde-welcome-shared OBJECT
    "src/mainwidget.cpp"
    "src/mainwidget.h"
    "src/updatecontent.cpp"
    "src/updatecontent.h"
    "src/utils.cpp"
    "src/utils.h"
)

target_link_libraries(dde-welcome-shared
    session-ui-widgets-shared
    session-ui-dbus-shared
    Dtk::Widget
    PkgConfig::GSETTINGS
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
)

add_subdirectory("src")
add_subdirectory("tests")
