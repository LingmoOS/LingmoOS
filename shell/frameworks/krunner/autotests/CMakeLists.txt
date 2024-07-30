# SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMAddTests)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

ecm_add_tests(
    dbusrunnertest.cpp
    runnermatchmethodstest.cpp
    runnermanagerhistorytest.cpp
    runnermanagersinglerunnermodetest.cpp
    runnermanagertest.cpp
    testmetadataconversion.cpp
    threadingtest.cpp
    LINK_LIBRARIES Qt6::Gui Qt6::DBus Qt6::Test KF6::Runner KF6::ConfigCore
)

kcoreaddons_add_plugin(fakerunnerplugin SOURCES plugins/fakerunnerplugin.cpp INSTALL_NAMESPACE "krunnertest" STATIC)
target_link_libraries(fakerunnerplugin KF6Runner Qt6::Gui)

kcoreaddons_add_plugin(suspendedrunnerplugin SOURCES plugins/suspendedrunner.cpp INSTALL_NAMESPACE "krunnertest2" STATIC)
target_link_libraries(suspendedrunnerplugin KF6Runner)

kcoreaddons_target_static_plugins(runnermanagerhistorytest NAMESPACE krunnertest)
kcoreaddons_target_static_plugins(runnermanagertest NAMESPACE krunnertest)
kcoreaddons_target_static_plugins(runnermanagertest NAMESPACE krunnertest2)
kcoreaddons_target_static_plugins(threadingtest NAMESPACE krunnertest)

add_executable(testremoterunner)
qt_add_dbus_adaptor(demoapp_dbus_adaptor_SRCS "../src/data/org.kde.krunner1.xml" plugins/testremoterunner.h TestRemoteRunner)
target_sources(testremoterunner PRIVATE plugins/testremoterunner.cpp ${demoapp_dbus_adaptor_SRCS})
target_link_libraries(testremoterunner
    Qt6::DBus
    Qt6::Gui
    KF6::Runner
)

include(../KF6KRunnerMacros.cmake)
krunner_configure_test(dbusrunnertest testremoterunner DESKTOP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/plugins/dbusrunnertest.desktop")
krunner_configure_test(runnermanagersinglerunnermodetest testremoterunner DESKTOP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/plugins/dbusrunnertest.desktop")
krunner_configure_test(runnermanagertest testremoterunner DESKTOP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/plugins/dbusrunnertest.desktop")
krunner_configure_test(threadingtest testremoterunner DESKTOP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/plugins/dbusrunnertest.desktop")

find_package(Qt6 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Widgets)
if (TARGET Qt6::Widgets)
    add_executable(modelwidgettest modelwidgettest.cpp)
    target_link_libraries(modelwidgettest KF6::Runner Qt6::Widgets)
    kcoreaddons_target_static_plugins(modelwidgettest NAMESPACE krunnertest)

endif()
add_executable(pluginbenchmarker pluginbenchmarker.cpp)
target_link_libraries(pluginbenchmarker KF6::Runner)
