add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_l2tpui\")



add_library(plasmanetworkmanagement_l2tpui MODULE)
target_sources(plasmanetworkmanagement_l2tpui PRIVATE
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
ki18n_wrap_ui(plasmanetworkmanagement_l2tpui l2tp.ui l2tpipsec.ui l2tpppp.ui)

target_link_libraries(plasmanetworkmanagement_l2tpui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS plasmanetworkmanagement_l2tpui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
