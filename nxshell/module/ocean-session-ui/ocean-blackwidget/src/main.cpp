// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"

#include <signal.h>
#include <DLog>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QTimer>

const QString DBUS_SERV = "org.lingmo.ocean.BlackScreen1";
const QString DBUS_PATH = "/org/lingmo/ocean/BlackScreen1";
const QString DBUS_IFCE = "org.lingmo.ocean.BlackScreen1";
const QString KWIN_INTERFACE_NAME = "org.kde.KWin";

bool onPreparingForShutdown() {
    QDBusInterface iface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus()
    );
    QVariant preparingForShutdown = iface.property("PreparingForShutdown");

    if (preparingForShutdown.isValid()) {
        bool isPreparing = preparingForShutdown.toBool();
        qDebug() << "Preparing for shutdown property value:" << isPreparing;
        return isPreparing;
    } else {
        qWarning() << "Failed to retrieve preparing for shutdown property, the property is invalid";
    }
    return false;
}

void handleSIGTERM(int signal) {
    qInfo() << "handleSIGTERM: " << signal;

    bool bShutdown = onPreparingForShutdown();
    qInfo() << "Whether preparing for shutdown: " << bShutdown;
    if (bShutdown) {
        QTimer::singleShot(2500, qApp, SLOT(quit()));
    } else {
        QTimer time;
        time.start(1000);
        QObject::connect(&time, &QTimer::timeout, [&] {
            bool bShutdown = onPreparingForShutdown();
            qInfo() << "Whether preparing for shutdown: " << bShutdown;
            if (bShutdown) {
                time.stop();
                QTimer::singleShot(2000, qApp, SLOT(quit()));
                return;
            } else {
                qInfo() << " Get org.freedesktop.login1.Manager PreparingForShutdown again.";
            }
        });
    }
}

int main(int argc, char *argv[])
{
    // ocean-blackwidet should not try reconnect when kwin_wayland crash
    qunsetenv("QT_WAYLAND_RECONNECT");

    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        auto connection = QDBusConnection::sessionBus();
        // should not start when kwin offline
        if (!connection.interface()->isServiceRegistered(KWIN_INTERFACE_NAME)) {
            qDebug() << "blackwidget return 0, for dbus not registered: " << KWIN_INTERFACE_NAME;
            return 0;
        }
    }

    QApplication a(argc, argv);

    DCORE_USE_NAMESPACE::Dtk::Core::DLogManager::registerConsoleAppender();

    // 默认日志路径是 ~/.cache/ocean-blackwidget/ocean-blackwidget.log
    DCORE_USE_NAMESPACE::Dtk::Core::DLogManager::registerFileAppender();
    DCORE_USE_NAMESPACE::Dtk::Core::DLogManager::registerJournalAppender();

    signal(SIGTERM, handleSIGTERM);
    bool useDBus = true;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.process(a); 
    int size = parser.positionalArguments().size();
    for (int i = 0; i < size; i++) {
        QString argument = parser.positionalArguments()[i];
        if (argument == "nodbus") {
            useDBus = false;
            break;
        }
    }

    Window w;
    w.setTimer();
    qInfo() << " Black screen use dbus : " << useDBus;

    if (!useDBus) {
        qInfo() << "Direct to launch black widget.";
        w.raiseWindow();
        return a.exec();
    }

    QDBusConnection sessionDBus = QDBusConnection::sessionBus();
    bool result = sessionDBus.interface()->registerService(DBUS_SERV,
                                                           QDBusConnectionInterface::ReplaceExistingService,
                                                           QDBusConnectionInterface::AllowReplacement);
    qInfo() << "Session register service : " << result;

    BlackWidgetAdaptor adaptor(&w);

    if (result) {
        qInfo() << "Session register object : " << sessionDBus.registerObject("/org/lingmo/ocean/BlackScreen1", &w);
        QObject::connect(sessionDBus.interface(), &QDBusConnectionInterface::serviceUnregistered, &w, &Window::onNameLost);
    } else {
        qInfo() << "End black widget.";
        return -1;
    }

    return a.exec();
}
