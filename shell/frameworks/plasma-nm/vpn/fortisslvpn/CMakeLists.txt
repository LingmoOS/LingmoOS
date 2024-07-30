add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_fortisslvpnui\")



add_library(plasmanetworkmanagement_fortisslvpnui MODULE)
target_sources(plasmanetworkmanagement_fortisslvpnui PRIVATE
    fortisslvpn.cpp
    fortisslvpnwidget.cpp
    fortisslvpnauth.cpp
    fortisslvpn.h
    fortisslvpnwidget.h
    fortisslvpnauth.h
)
ki18n_wrap_ui(plasmanetworkmanagement_fortisslvpnui fortisslvpn.ui fortisslvpnadvanced.ui fortisslvpnauth.ui)

target_link_libraries(plasmanetworkmanagement_fortisslvpnui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS plasmanetworkmanagement_fortisslvpnui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
