# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

set(KSYSGUARD_OSINFO_PLUGIN_SOURCES
    osinfo.cpp
)

add_library(ksystemstats_plugin_osinfo MODULE ${KSYSGUARD_OSINFO_PLUGIN_SOURCES})
target_link_libraries(ksystemstats_plugin_osinfo Qt::Core Qt::Gui Qt::DBus KF6::CoreAddons KF6::I18n KSysGuard::SystemStats)

install(TARGETS ksystemstats_plugin_osinfo DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
