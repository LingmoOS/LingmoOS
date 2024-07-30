add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_libreswanui\")



add_library(lingmonetworkmanagement_libreswanui MODULE)
target_sources(lingmonetworkmanagement_libreswanui PRIVATE
    libreswan.cpp
    libreswanwidget.cpp
    libreswanauth.cpp
    libreswan.h
    libreswanwidget.h
    libreswanauth.h
)
ki18n_wrap_ui(lingmonetworkmanagement_libreswanui libreswan.ui libreswanauth.ui)

target_link_libraries(lingmonetworkmanagement_libreswanui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_libreswanui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
