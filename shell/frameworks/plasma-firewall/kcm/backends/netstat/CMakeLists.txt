# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    Xml
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    Auth
    Config
)


add_library(networkstatus STATIC)
set_property(TARGET networkstatus PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(networkstatus PRIVATE
    connectionsmodel.cpp
    connectionsmodel.h
    netstatclient.cpp
    netstatclient.h
    netstathelper.cpp
    netstathelper.h
)

target_link_libraries(networkstatus
    kcm_firewall_core
    Qt::Core
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::AuthCore
    KF6::I18n
)
