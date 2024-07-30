add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_openconnectui\")

ecm_qt_declare_logging_category(openvpn_plugins_SRCS HEADER plasma_nm_openconnect.h IDENTIFIER PLASMA_NM_OPENCONNECT_LOG CATEGORY_NAME org.kde.plasma.nm.openconnect DESCRIPTION "Plasma NM (openconnect plugins)" EXPORT PLASMANM)

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

    add_library(plasmanetworkmanagement_openconnect_${name} MODULE ${openconnect_SRCS})

    target_link_libraries(plasmanetworkmanagement_openconnect_${name}
        plasmanm_internal
        plasmanm_editor
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

    install(TARGETS plasmanetworkmanagement_openconnect_${name}  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
endmacro()

add_openconnect_plugin(anyconnect)
add_openconnect_plugin(juniperui)
add_openconnect_plugin(globalprotectui)
add_openconnect_plugin(pulseui)
add_openconnect_plugin(arrayui)
add_openconnect_plugin(f5ui)
add_openconnect_plugin(fortinetui)
