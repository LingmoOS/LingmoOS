kcoreaddons_add_plugin(org.kde.lingmo.systemmonitor SOURCES systemmonitor.cpp systemmonitor.h INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.systemmonitor
                      Qt::Gui
                      Qt::Qml
                      Qt::Quick
                      Qt::DBus
                      Lingmo::Lingmo
                      Lingmo::LingmoQuick
                      KF6::I18n
                      KF6::ConfigCore
                      KF6::ConfigGui
                      KF6::KIOGui
                      KF6::JobWidgets
                      KSysGuard::Sensors
                      KSysGuard::SensorFaces
                     )

lingmo_install_package(package org.kde.lingmo.systemmonitor)
