add_library(KF6IdleTimeXcbPlugin1 MODULE)

set(xscreensaver_plugin_dbus_SRCS)
qt_add_dbus_interface(xscreensaver_plugin_dbus_SRCS org.freedesktop.ScreenSaver.xml screensaver_interface)

target_sources(KF6IdleTimeXcbPlugin1 PRIVATE
    xscreensaverbasedpoller.cpp
    ${xscreensaver_plugin_dbus_SRCS}
)

target_link_libraries(KF6IdleTimeXcbPlugin1
    KF6IdleTime
    Qt6::DBus
    Qt6::Gui
    ${X11_LIBRARIES}
    ${X11_Xscreensaver_LIB}
)

install(
    TARGETS
        KF6IdleTimeXcbPlugin1
    DESTINATION
        ${KDE_INSTALL_PLUGINDIR}/kf6/org.kde.kidletime.platforms/
)
