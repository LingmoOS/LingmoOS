add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_vpncui\")

add_library(lingmonetworkmanagement_vpncui MODULE)
target_sources(lingmonetworkmanagement_vpncui PRIVATE
    vpnc.cpp
    vpncwidget.cpp
    vpncadvancedwidget.cpp
    vpncauth.cpp
    vpnc.h
    vpncwidget.h
    vpncadvancedwidget.h
    vpncauth.h
)
ecm_qt_declare_logging_category(lingmonetworkmanagement_vpncui HEADER lingmo_nm_vpnc.h IDENTIFIER LINGMO_NM_VPNC_LOG CATEGORY_NAME org.kde.lingmo.nm.openvpn DESCRIPTION "Lingmo NM (vpnc plugins)" EXPORT LINGMONM)

ki18n_wrap_ui(lingmonetworkmanagement_vpncui vpnc.ui vpncadvanced.ui vpncauth.ui)

target_link_libraries(lingmonetworkmanagement_vpncui
    lingmonm_internal
    lingmonm_editor
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::KIOWidgets
    KF6::I18n
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_vpncui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
