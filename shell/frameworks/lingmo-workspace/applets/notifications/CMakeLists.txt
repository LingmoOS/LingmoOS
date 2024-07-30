add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.notifications\")

set(notificationapplet_SRCS
    notificationapplet.cpp
    draghelper.cpp
    fileinfo.cpp
    filemenu.cpp
    globalshortcuts.cpp
    jobaggregator.cpp
    thumbnailer.cpp
    wheelforwarder.cpp
    notificationapplet.h
    draghelper.h
    fileinfo.h
    filemenu.h
    globalshortcuts.h
    jobaggregator.h
    thumbnailer.h
)

kcoreaddons_add_plugin(org.kde.lingmo.notifications SOURCES ${notificationapplet_SRCS} INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.notifications
                      Qt::Gui
                      Qt::Quick # for QQmlParserStatus
                      KF6::ConfigWidgets # for KStandardAction
                      KF6::I18n
                      Lingmo::Lingmo
                      Lingmo::LingmoQuick
                      KF6::CoreAddons
                      KF6::GlobalAccel
                      KF6::KIOGui
                      KF6::KIOWidgets # for PreviewJob
                      KF6::Notifications # for KNotificationJobUiDelegate
                      PW::LibNotificationManager
                      )

ecm_qt_declare_logging_category(org.kde.lingmo.notifications
    HEADER notifications_debug.h
    IDENTIFIER LINGMO_APPLET_NOTIFICATIONS_DEBUG
    CATEGORY_NAME org.kde.lingmo.notifications
    DESCRIPTION "lingmo applet notification"
    EXPORT LINGMOWORKSPACE
)

lingmo_install_package(package org.kde.lingmo.notifications)
