add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_iodineui\")



add_library(lingmonetworkmanagement_iodineui MODULE)
target_sources(lingmonetworkmanagement_iodineui PRIVATE
    iodine.cpp
    iodinewidget.cpp
    iodineauth.cpp
    iodine.h
    iodinewidget.h
    iodineauth.h
)
ki18n_wrap_ui(lingmonetworkmanagement_iodineui iodine.ui iodineauth.ui)

target_link_libraries(lingmonetworkmanagement_iodineui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_iodineui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
