add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_openvpnui\")

# Needed to build this module
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)



add_library(plasmanetworkmanagement_openvpnui MODULE)
target_sources(plasmanetworkmanagement_openvpnui PRIVATE
    openvpn.cpp
    openvpnwidget.cpp
    openvpnauth.cpp
    openvpnadvancedwidget.cpp
    openvpn.h
    openvpnwidget.h
    openvpnauth.h
    openvpnadvancedwidget.h
)
ecm_qt_declare_logging_category(plasmanetworkmanagement_openvpnui HEADER plasma_nm_openvpn.h IDENTIFIER PLASMA_NM_OPENVPN_LOG CATEGORY_NAME org.kde.plasma.nm.openvpn DESCRIPTION "Plasma NM (openvpn plugins)" EXPORT PLASMANM)
ki18n_wrap_ui(plasmanetworkmanagement_openvpnui openvpn.ui openvpnadvanced.ui)

target_link_libraries(plasmanetworkmanagement_openvpnui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
    KF6::KIOWidgets
)

install(TARGETS plasmanetworkmanagement_openvpnui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
