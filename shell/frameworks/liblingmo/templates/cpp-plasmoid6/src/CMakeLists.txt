# TODO: adapt "org.kde.lingmo" here & elsewhere if needed (see README)
add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.%{APPNAMELC}\")

add_library(org.kde.lingmo.%{APPNAMELC} MODULE %{APPNAMELC}.cpp)

target_link_libraries(org.kde.lingmo.%{APPNAMELC}
                      Qt6::Gui
                      Lingmo::Lingmo
                      KF6::I18n)


install(TARGETS org.kde.lingmo.%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo/applets)

lingmo_install_package(package org.kde.lingmo.%{APPNAMELC})
