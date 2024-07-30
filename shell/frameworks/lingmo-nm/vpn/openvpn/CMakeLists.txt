add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_openvpnui\")

# Needed to build this module
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)



add_library(lingmonetworkmanagement_openvpnui MODULE)
target_sources(lingmonetworkmanagement_openvpnui PRIVATE
    openvpn.cpp
    openvpnwidget.cpp
    openvpnauth.cpp
    openvpnadvancedwidget.cpp
    openvpn.h
    openvpnwidget.h
    openvpnauth.h
    openvpnadvancedwidget.h
)
ecm_qt_declare_logging_category(lingmonetworkmanagement_openvpnui HEADER lingmo_nm_openvpn.h IDENTIFIER LINGMO_NM_OPENVPN_LOG CATEGORY_NAME org.kde.lingmo.nm.openvpn DESCRIPTION "Lingmo NM (openvpn plugins)" EXPORT LINGMONM)
ki18n_wrap_ui(lingmonetworkmanagement_openvpnui openvpn.ui openvpnadvanced.ui)

target_link_libraries(lingmonetworkmanagement_openvpnui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
    KF6::KIOWidgets
)

install(TARGETS lingmonetworkmanagement_openvpnui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
