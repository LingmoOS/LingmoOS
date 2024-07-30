add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_l2tpui\")



add_library(lingmonetworkmanagement_l2tpui MODULE)
target_sources(lingmonetworkmanagement_l2tpui PRIVATE
    l2tp.cpp
    l2tpwidget.cpp
    l2tpauth.cpp
    l2tpipsecwidget.cpp
    l2tppppwidget.cpp
    l2tp.h
    l2tpwidget.h
    l2tpauth.h
    l2tpipsecwidget.h
    l2tppppwidget.h
)
ki18n_wrap_ui(lingmonetworkmanagement_l2tpui l2tp.ui l2tpipsec.ui l2tpppp.ui)

target_link_libraries(lingmonetworkmanagement_l2tpui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_l2tpui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
