add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_sstpui\")



add_library(lingmonetworkmanagement_sstpui MODULE)
target_sources(lingmonetworkmanagement_sstpui PRIVATE
   sstp.cpp
   sstpwidget.cpp
   sstpauth.cpp
   sstp.h
   sstpwidget.h
   sstpauth.h
   )
ki18n_wrap_ui(lingmonetworkmanagement_sstpui sstpwidget.ui sstpadvanced.ui sstpauth.ui)

target_link_libraries(lingmonetworkmanagement_sstpui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_sstpui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
