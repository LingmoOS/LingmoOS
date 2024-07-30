# SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
# SPDX-License-Identifier: BSD-2-Clause

set(SOURCES
    main.cpp
)

add_executable(kstatsviewer ${SOURCES})
target_link_libraries(kstatsviewer Qt::Core Qt::DBus KSysGuard::Formatter KSysGuard::SystemStats)

install(TARGETS kstatsviewer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
