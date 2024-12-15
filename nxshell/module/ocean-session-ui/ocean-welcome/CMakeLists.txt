# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ocean-welcome-shared OBJECT
    "src/mainwidget.cpp"
    "src/mainwidget.h"
    "src/updatecontent.cpp"
    "src/updatecontent.h"
    "src/utils.cpp"
    "src/utils.h"
)

target_link_libraries(ocean-welcome-shared
    session-ui-widgets-shared
    session-ui-dbus-shared
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
)

add_subdirectory("src")
add_subdirectory("tests")
