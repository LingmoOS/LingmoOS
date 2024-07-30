add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_strongswanui\")



add_library(lingmonetworkmanagement_strongswanui MODULE)
target_sources(lingmonetworkmanagement_strongswanui PRIVATE
   strongswan.cpp
   strongswanwidget.cpp
   strongswanauth.cpp
   strongswan.h
   strongswanwidget.h
   strongswanauth.h
   )
ki18n_wrap_ui(lingmonetworkmanagement_strongswanui strongswanprop.ui strongswanauth.ui)

target_link_libraries(lingmonetworkmanagement_strongswanui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
)

install(TARGETS lingmonetworkmanagement_strongswanui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
