# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

add_definitions(-DTRANSLATION_DOMAIN=\"ksystemstats_plugins\")

set(KSYSTEMSTATS_PLUGIN_INSTALL_DIR ${KDE_INSTALL_PLUGINDIR}/ksystemstats)

add_subdirectory(osinfo)
add_subdirectory(network)
add_subdirectory(power)
add_subdirectory(disks)
add_subdirectory(cpu)
add_subdirectory(memory)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(pressure)
endif ()

if(UDev_FOUND)
    add_subdirectory(gpu)
endif()

if(SENSORS_FOUND)
    add_subdirectory(lmsensors)
endif()
