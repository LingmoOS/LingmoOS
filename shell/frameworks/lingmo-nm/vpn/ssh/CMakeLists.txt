add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement_sshui\")



add_library(lingmonetworkmanagement_sshui MODULE)
target_sources(lingmonetworkmanagement_sshui PRIVATE
   ssh.cpp
   sshwidget.cpp
   sshauth.cpp
   ssh.h
   sshwidget.h
   sshauth.h
   )
ki18n_wrap_ui(lingmonetworkmanagement_sshui sshwidget.ui sshadvanced.ui sshauth.ui)

target_link_libraries(lingmonetworkmanagement_sshui
    lingmonm_internal
    lingmonm_editor
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)

install(TARGETS lingmonetworkmanagement_sshui  DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/network/vpn)
