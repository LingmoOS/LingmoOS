# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

add_library(ksystemstats_plugin_cpu MODULE  cpu.cpp cpuplugin.cpp loadaverages.cpp usagecomputer.cpp)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(ksystemstats_plugin_cpu PRIVATE linuxcpu.cpp linuxcpuplugin.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_sources(ksystemstats_plugin_cpu PRIVATE freebsdcpuplugin.cpp)
endif()

target_link_libraries(ksystemstats_plugin_cpu Qt::Core KF6::CoreAddons KF6::I18n KSysGuard::SystemStats)

install(TARGETS ksystemstats_plugin_cpu DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
