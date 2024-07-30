# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

include(GenerateExportHeader)

set(core_files
    appprofiles.cpp
    appprofiles.h
    firewallclient.cpp
    firewallclient.h
    ifirewallclientbackend.cpp
    ifirewallclientbackend.h
    ipvalidator.cpp
    ipvalidator.h
    loglistmodel.cpp
    loglistmodel.h
    profile.cpp
    profile.h
    rule.cpp
    rule.h
    rulelistmodel.cpp
    rulelistmodel.h
    types.cpp
    types.h
)

set(core_link_libraries
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::AuthCore
)

# insert additional linux specific systemdjob files
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set (core_files ${core_files} systemdjob.h systemdjob.cpp)
    set (core_link_libraries ${core_link_libraries} Qt::DBus)
endif()

add_library(kcm_firewall_core ${core_files})
target_link_libraries(kcm_firewall_core ${core_link_libraries})

generate_export_header(kcm_firewall_core)

target_include_directories(kcm_firewall_core PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")


install(TARGETS kcm_firewall_core ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
