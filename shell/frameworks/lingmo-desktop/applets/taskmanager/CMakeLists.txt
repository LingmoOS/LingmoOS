add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.taskmanager\")

lingmo_install_package(package org.kde.lingmo.taskmanager)

set(taskmanagerplugin_SRCS
    plugin/backend.cpp
    plugin/taskmanagerplugin.cpp

    plugin/smartlaunchers/smartlauncherbackend.cpp
    plugin/smartlaunchers/smartlauncheritem.cpp
)

ecm_qt_declare_logging_category(taskmanagerplugin_SRCS
    HEADER log_settings.h
    IDENTIFIER TASKMANAGER_DEBUG
    CATEGORY_NAME org.kde.lingmo.taskmanager)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/taskmanager)

add_library(taskmanagerplugin SHARED ${taskmanagerplugin_SRCS})

# FIXME Cleanup no longer used libs.
target_link_libraries(taskmanagerplugin
                      Qt::Core
                      Qt::DBus
                      Qt::Qml
                      Qt::Quick
                      Lingmo::Activities
                      Lingmo::ActivitiesStats
                      KF6::I18n
                      KF6::KIOCore
                      KF6::KIOGui
                      KF6::KIOFileWidgets # KFilePlacesModel
                      KF6::Notifications # KNotificationJobUiDelegate
                      Lingmo::Lingmo
                      KSysGuard::ProcessCore
                      KF6::Service
                      KF6::WindowSystem
                      PW::LibNotificationManager)

install(TARGETS taskmanagerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/taskmanager)
