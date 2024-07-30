add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_sstpui\")



add_library(plasmanetworkmanagement_sstpui MODULE)
target_sources(plasmanetworkmanagement_sstpui PRIVATE
   sstp.cpp
   sstpwidget.cpp
   sstpauth.cpp
   sstp.h
   sstpwidget.h
   sstpauth.h
   )
ki18n_wrap_ui(plasmanetworkmanagement_sstpui sstpwidget.ui sstpadvanced.ui sstpauth.ui)

target_link_libraries(plasmanetworkmanagement_sstpui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS plasmanetworkmanagement_sstpui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
