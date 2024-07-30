add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_containmentactions_contextmenu\")

include_directories(${lingmo-workspace_SOURCE_DIR}/libkworkspace)

set(contextmenu_SRCS
    menu.cpp
    menu.h
)

set(krunner_xml ${lingmo-workspace_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
qt_add_dbus_interface(contextmenu_SRCS ${krunner_xml} krunner_interface)

qt_add_dbus_interface(contextmenu_SRCS ${SCREENSAVER_DBUS_INTERFACE} screensaver_interface)

kcoreaddons_add_plugin(lingmo_containmentactions_contextmenu SOURCES ${contextmenu_SRCS} INSTALL_NAMESPACE "lingmo/containmentactions")

set_target_properties(lingmo_containmentactions_contextmenu PROPERTIES
    OUTPUT_NAME org.kde.contextmenu
)

target_link_libraries(lingmo_containmentactions_contextmenu
                      Qt::DBus
                      Qt::Qml
                      Lingmo::Activities
                      KF6::I18n
                      KF6::GlobalAccel
                      Lingmo::Lingmo
                      KF6::KIOCore
                      KF6::KIOGui
                      PW::KWorkspace)
