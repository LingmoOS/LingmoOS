# SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
# SPDX-License-Identifier: BSD-2-Clause

set(SOURCES
    client.cpp
    daemon.cpp
)

find_file(SYSTEMSTATS_DBUS_INTERFACE NAMES dbus-1/interfaces/org.kde.ksystemstats1.xml HINTS ${KDE_INSTALL_FULL_DATADIR} PATH_SUFFIXES ${KDE_INSTALL_DATADIR})
qt_add_dbus_adaptor(SOURCES ${SYSTEMSTATS_DBUS_INTERFACE} daemon.h Daemon)

add_library(ksystemstats_core STATIC ${SOURCES})
target_link_libraries(ksystemstats_core PUBLIC Qt::Core Qt::DBus KF6::CoreAddons KSysGuard::SystemStats)

add_executable(ksystemstats main.cpp)
target_link_libraries(ksystemstats ksystemstats_core)

install(TARGETS ksystemstats DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_dbus_service_file(
    NAME org.kde.ksystemstats1
    EXECUTABLE "${KDE_INSTALL_FULL_BINDIR}/ksystemstats"
    SYSTEMD_SERVICE plasma-ksystemstats.service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

ecm_install_configured_files(
   INPUT plasma-ksystemstats.service.in
   DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)
