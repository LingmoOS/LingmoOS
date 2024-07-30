# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

add_library(ksystemstats_plugin_power MODULE power.cpp)
target_link_libraries(ksystemstats_plugin_power KF6::CoreAddons KF6::I18n KF6::Solid KSysGuard::SystemStats)
install(TARGETS ksystemstats_plugin_power DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
