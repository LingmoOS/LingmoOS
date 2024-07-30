lingmo_install_package(package org.kde.lingmo.showdesktop)

add_definitions(-DTRANSLATION_DOMAIN="lingmo_applet_org.kde.lingmo.showdesktop")

set(showdesktop_SRCS
    plugin/showdesktop.cpp
    plugin/showdesktopplugin.cpp
)

qt_add_dbus_interface(showdesktop_SRCS plugin/org.kde.KWin.xml kwin_interface)

add_library(showdesktopplugin SHARED ${showdesktop_SRCS})

target_link_libraries(showdesktopplugin
     Qt::Core
     Qt::DBus
     Qt::Qml
     Qt::Quick
     KF6::WindowSystem
    )

install(TARGETS showdesktopplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/showdesktop)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/showdesktop)
