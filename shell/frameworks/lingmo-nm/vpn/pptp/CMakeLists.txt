add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_pptpui\")


add_library(lingmonetworkmanagement_pptpui MODULE)
target_sources(lingmonetworkmanagement_pptpui PRIVATE
   pptp.cpp
   pptpwidget.cpp
   pptpauth.cpp
   pptp.h
   pptpwidget.h
   pptpauth.h
   )
ki18n_wrap_ui(lingmonetworkmanagement_pptpui pptpprop.ui pptpadvanced.ui pptpauth.ui)

target_link_libraries(lingmonetworkmanagement_pptpui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_pptpui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
