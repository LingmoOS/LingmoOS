add_definitions(-DTRANSLATION_DOMAIN=\"plasma_containmentactions_contextmenu\")

include_directories(${lingmo-workspace_SOURCE_DIR}/libkworkspace)

set(contextmenu_SRCS
    menu.cpp
    menu.h
)

set(krunner_xml ${lingmo-workspace_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
qt_add_dbus_interface(contextmenu_SRCS ${krunner_xml} krunner_interface)

qt_add_dbus_interface(contextmenu_SRCS ${SCREENSAVER_DBUS_INTERFACE} screensaver_interface)

kcoreaddons_add_plugin(plasma_containmentactions_contextmenu SOURCES ${contextmenu_SRCS} INSTALL_NAMESPACE "plasma/containmentactions")

target_link_libraries(plasma_containmentactions_contextmenu
                      Qt::DBus
                      KF5::Activities
                      KF5::I18n
                      KF5::GlobalAccel
                      KF5::Plasma
                      KF5::XmlGui
                      KF5::KIOCore
                      KF5::KIOGui
                      PW::KWorkspace)
