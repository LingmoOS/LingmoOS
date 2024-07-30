add_subdirectory(kwin_bridge)
add_subdirectory(scss)

add_library(gtkconfig MODULE)

set(gtkconfig_SRCS
    gtkconfig.cpp
    gsd-xsettings-manager/gsd-xsettings-manager.cpp
    configvalueprovider.cpp
    themepreviewer.cpp
)

qt_add_dbus_adaptor(gtkconfig_SRCS
    gsd-xsettings-manager/org.gtk.Settings.xml
    gsd-xsettings-manager/gsd-xsettings-manager.h
    GSDXSettingsManager
    gsd_xsettings_manager_adaptor
    GSDXSettingsManagerAdaptor
)

target_sources(gtkconfig
  PRIVATE
    ${gtkconfig_SRCS}
)

add_subdirectory(config_editor)

target_compile_definitions(gtkconfig
  PUBLIC
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

target_link_libraries(gtkconfig
  PRIVATE
    KWinBridge
    KF6::WindowSystem
  PUBLIC
    Qt::DBus
    Qt::Svg
    KF6::ColorScheme
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::DBusAddons
    KF6::GuiAddons
    PkgConfig::GTK+3
    PkgConfig::GIO
    PkgConfig::GObject
)

install(TARGETS gtkconfig DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)
