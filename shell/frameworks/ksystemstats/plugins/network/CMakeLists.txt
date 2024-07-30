# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
# SPDX-FileCopyrightText: 2021 Alessio Bonfiglio <alessio.bonfiglio@mail.polimi.it>

set(KSYSGUARD_NETWORK_PLUGIN_SOURCES
    NetworkPlugin.cpp
    NetworkDevice.cpp
    NetworkBackend.cpp
    AllDevicesObject.cpp
)

if (KF6NetworkManagerQt_FOUND)
    set(KSYSGUARD_NETWORK_PLUGIN_SOURCES ${KSYSGUARD_NETWORK_PLUGIN_SOURCES} NetworkManagerBackend.cpp)
    add_definitions(-DNETWORKMANAGER_FOUND)
endif()

add_library(ksystemstats_plugin_network MODULE ${KSYSGUARD_NETWORK_PLUGIN_SOURCES})
target_link_libraries(ksystemstats_plugin_network PRIVATE Qt::Core Qt::Gui Qt::DBus KF6::CoreAddons KF6::I18n KSysGuard::SystemStats)

if (KF6NetworkManagerQt_FOUND)
    target_link_libraries(ksystemstats_plugin_network PRIVATE KF6::NetworkManagerQt)
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources(ksystemstats_plugin_network PRIVATE RtNetlinkBackend.cpp)
    target_link_libraries(ksystemstats_plugin_network PRIVATE ${NL_LIBRARIES} Qt::Network)
    target_include_directories(ksystemstats_plugin_network PRIVATE ${NL_INCLUDE_DIRS})
endif()

install(TARGETS ksystemstats_plugin_network DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
