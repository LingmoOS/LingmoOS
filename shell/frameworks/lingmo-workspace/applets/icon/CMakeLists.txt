add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.icon\")

kcoreaddons_add_plugin(org.kde.lingmo.icon SOURCES iconapplet.cpp iconapplet.h INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.icon
                      KF6::I18n
                      KF6::KIOCore # for OpenFileManagerWindowJob
                      KF6::KIOGui # for FavIconRequestJob
                      KF6::KIOWidgets # for KRun
                      KF6::Notifications
                      Lingmo::Lingmo
                      PW::LibTaskManager)

lingmo_install_package(package org.kde.lingmo.icon)
