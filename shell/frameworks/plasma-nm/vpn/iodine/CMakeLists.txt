add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_iodineui\")



add_library(plasmanetworkmanagement_iodineui MODULE)
target_sources(plasmanetworkmanagement_iodineui PRIVATE
    iodine.cpp
    iodinewidget.cpp
    iodineauth.cpp
    iodine.h
    iodinewidget.h
    iodineauth.h
)
ki18n_wrap_ui(plasmanetworkmanagement_iodineui iodine.ui iodineauth.ui)

target_link_libraries(plasmanetworkmanagement_iodineui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
)

install(TARGETS plasmanetworkmanagement_iodineui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
