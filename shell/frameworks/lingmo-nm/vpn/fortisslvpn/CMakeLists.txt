add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_fortisslvpnui\")



add_library(lingmonetworkmanagement_fortisslvpnui MODULE)
target_sources(lingmonetworkmanagement_fortisslvpnui PRIVATE
    fortisslvpn.cpp
    fortisslvpnwidget.cpp
    fortisslvpnauth.cpp
    fortisslvpn.h
    fortisslvpnwidget.h
    fortisslvpnauth.h
)
ki18n_wrap_ui(lingmonetworkmanagement_fortisslvpnui fortisslvpn.ui fortisslvpnadvanced.ui fortisslvpnauth.ui)

target_link_libraries(lingmonetworkmanagement_fortisslvpnui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_fortisslvpnui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
