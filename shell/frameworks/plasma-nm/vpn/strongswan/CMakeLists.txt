add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement_strongswanui\")



add_library(plasmanetworkmanagement_strongswanui MODULE)
target_sources(plasmanetworkmanagement_strongswanui PRIVATE
   strongswan.cpp
   strongswanwidget.cpp
   strongswanauth.cpp
   strongswan.h
   strongswanwidget.h
   strongswanauth.h
   )
ki18n_wrap_ui(plasmanetworkmanagement_strongswanui strongswanprop.ui strongswanauth.ui)

target_link_libraries(plasmanetworkmanagement_strongswanui
    plasmanm_internal
    plasmanm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
)

install(TARGETS plasmanetworkmanagement_strongswanui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/network/vpn)
