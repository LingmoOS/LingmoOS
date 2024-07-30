# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

add_library(ksystemstats_plugin_disk MODULE  disks.cpp)
target_link_libraries(ksystemstats_plugin_disk Qt::Core KF6::CoreAddons KF6::I18n KF6::KIOCore KF6::Solid KSysGuard::SystemStats)

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_link_libraries(ksystemstats_plugin_disk geom devstat)
endif()

install(TARGETS ksystemstats_plugin_disk DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
