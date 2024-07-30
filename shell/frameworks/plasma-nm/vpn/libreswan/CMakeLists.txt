add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_libreswanui\")



add_library(plasmanetworkmanagement_libreswanui MODULE)
target_sources(plasmanetworkmanagement_libreswanui PRIVATE
    libreswan.cpp
    libreswanwidget.cpp
    libreswanauth.cpp
    libreswan.h
    libreswanwidget.h
    libreswanauth.h
)
ki18n_wrap_ui(plasmanetworkmanagement_libreswanui libreswan.ui libreswanauth.ui)

target_link_libraries(plasmanetworkmanagement_libreswanui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
)

install(TARGETS plasmanetworkmanagement_libreswanui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
