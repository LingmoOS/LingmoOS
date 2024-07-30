# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_disks\")

set(kded_SRCS
    smartdata.cpp smartdata.h
    smartmonitor.cpp smartmonitor.h
    smartctl.cpp smartctl.h
    smartnotifier.cpp smartnotifier.h
    dbusobjectmanagerserver.cpp
    device.cpp
    devicenotifier.cpp
    soliddevicenotifier.cpp soliddevicenotifier.h
    instabilities.cpp instabilities.h
    smartfailure.cpp smartfailure.h
)

if(WITH_SIMULATION)
    list(APPEND kded_SRCS
        simulationdevicenotifier.cpp simulationdevicenotifier.h
        simulationctl.cpp simulationctl.h
        ../autotests/fixtures/simulation.qrc)
    add_definitions(-DWITH_SIMULATION)
endif()

ecm_qt_declare_logging_category(
    kded_SRCS
    HEADER "kded_debug.h"
    IDENTIFIER "KDED"
    CATEGORY_NAME "org.kde.plasma.disks"
    DESCRIPTION "Plasma Disks"
    EXPORT PlasmaDisks
)

add_library(statickdedsmart STATIC ${kded_SRCS})
set_property(TARGET statickdedsmart PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(statickdedsmart
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_link_libraries(statickdedsmart
    PUBLIC
    Qt6::Core
    Qt6::DBus
    KF6::Solid
    KF6::I18n
    KF6::Notifications
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::Service
    KF6::KIOGui
    KF6::AuthCore
)

add_library(kded-smart MODULE main.cpp)
set_target_properties(kded-smart PROPERTIES OUTPUT_NAME smart)
target_link_libraries(kded-smart statickdedsmart)
install(TARGETS kded-smart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)

kauth_install_actions(org.kde.kded.smart org.kde.kded.smart.actions)
add_executable(kded-smart-helper helper.cpp)
target_link_libraries(kded-smart-helper KF6::AuthCore KF6::ConfigCore KF6::I18n)

kauth_install_helper_files(kded-smart-helper org.kde.kded.smart root)
install(TARGETS kded-smart-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

install(FILES org.kde.kded.smart.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

add_subdirectory(kcm)
