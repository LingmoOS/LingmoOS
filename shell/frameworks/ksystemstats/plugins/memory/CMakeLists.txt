# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    add_library(ksystemstats_plugin_memory MODULE memory.cpp backend.cpp)

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(ksystemstats_plugin_memory PRIVATE linuxbackend.cpp)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        target_sources(ksystemstats_plugin_memory PRIVATE freebsdbackend.cpp)
        target_link_libraries(ksystemstats_plugin_memory kvm)
    endif()

    target_link_libraries(ksystemstats_plugin_memory Qt::Core KF6::CoreAddons KF6::I18n KSysGuard::SystemStats)

    install(TARGETS ksystemstats_plugin_memory DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
endif()
