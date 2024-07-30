# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

find_package(Qt${QT_MAJOR_VERSION}Widgets CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/plugin)

set(passwordsmodeltest_SRCS
    main.cpp
    mainwindow.cpp
)

add_executable(passwordsmodeltest ${passwordsmodeltest_SRCS})
target_link_libraries(passwordsmodeltest
    plasmapass
    Qt::Core
    Qt::DBus
    Qt::Widgets
)
