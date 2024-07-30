# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

add_library(ksystemstats_plugin_lmsensors MODULE lmsensors.cpp)

target_link_libraries(ksystemstats_plugin_lmsensors PRIVATE Qt::Core KF6::CoreAddons KF6::I18n KSysGuard::SystemStats)

install(TARGETS ksystemstats_plugin_lmsensors DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})

