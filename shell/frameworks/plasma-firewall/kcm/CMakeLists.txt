# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_firewall\")
add_subdirectory(core)
add_subdirectory(backends)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

kcmutils_add_qml_kcm(kcm_firewall)
target_sources(kcm_firewall PRIVATE
   kcm.cpp
   kcm.h
)
target_link_libraries(kcm_firewall PRIVATE
    Qt::Quick
    Qt::DBus

    KF6::CoreAddons
    KF6::KCMUtils
    KF6::I18n
    KF6::AuthCore

    kcm_firewall_core
    networkstatus
)
