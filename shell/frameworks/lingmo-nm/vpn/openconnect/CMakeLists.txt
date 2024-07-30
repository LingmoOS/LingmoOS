add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_openconnectui\")

ecm_qt_declare_logging_category(openvpn_plugins_SRCS HEADER lingmo_nm_openconnect.h IDENTIFIER LINGMO_NM_OPENCONNECT_LOG CATEGORY_NAME org.kde.lingmo.nm.openconnect DESCRIPTION "Lingmo NM (openconnect plugins)" EXPORT LINGMONM)

macro(add_openconnect_plugin name)
    set(openconnect_SRCS
        ${openvpn_plugins_SRCS}
        openconnectui.cpp
        openconnectwidget.cpp
        openconnectauth.cpp
        openconnectauthworkerthread.cpp
        openconnectui.h
        openconnectwidget.h
        openconnectauth.h
        openconnectauthworkerthread.h
        openconnectwebauthdialog.cpp
        openconnectwebauthdialog.h
        openconnect_${name}.cpp
    )

    ki18n_wrap_ui(openconnect_SRCS
        openconnectprop.ui
        openconnectauth.ui
        openconnecttoken.ui
        openconnectwebauthdialog.ui
    )

    add_library(lingmonetworkmanagement_openconnect_${name} MODULE ${openconnect_SRCS})

    target_link_libraries(lingmonetworkmanagement_openconnect_${name}
        lingmonm_internal
        lingmonm_editor
        KF6::NetworkManagerQt
        KF6::CoreAddons
        KF6::I18n
        KF6::KIOWidgets
        KF6::WidgetsAddons
        Qt::WebEngineCore
        Qt::WebEngineWidgets
        Qt::Xml
        PkgConfig::OPENCONNECT
    )

    install(TARGETS lingmonetworkmanagement_openconnect_${name}  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
endmacro()

add_openconnect_plugin(anyconnect)
add_openconnect_plugin(juniperui)
add_openconnect_plugin(globalprotectui)
add_openconnect_plugin(pulseui)
add_openconnect_plugin(arrayui)
add_openconnect_plugin(f5ui)
add_openconnect_plugin(fortinetui)
