// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DLog>

#include <PolkitQt1/Subject>

#include "policykitlistener.h"
#include "accessible.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

#define APP_NAME "dde-polkit-agent"
#define APP_DISPLAY_NAME "Deepin Polkit Agent"
#define AUTH_DBUS_PATH "/com/deepin/dde/Polkit1/AuthAgent"

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setOrganizationName("deepin");
    a.setApplicationName(APP_NAME);
    a.setApplicationDisplayName(APP_DISPLAY_NAME);
    a.setApplicationVersion("0.1");
    a.setQuitOnLastWindowClosed(false);

    QAccessible::installFactory(accessibleFactory);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    if (!a.setSingleInstance(APP_NAME, DApplication::UserScope)) {
        qWarning() << "polkit is running!";
        return 0;
    }

    PolkitQt1::UnixSessionSubject session(getpid());
    PolicyKitListener listener;

    if (!listener.registerListener(session, AUTH_DBUS_PATH)) {
        qWarning() << "register listener failed!";
        return -1;
    }

    // create PID file to ~/.cache/deepin/dde-polkit-agent
    const QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();

    QDir dir(cachePath);
    if (!dir.exists()) {
        dir.mkpath(cachePath);
    }

    QFile PID(cachePath + QDir::separator() + "pid");
    if (PID.open(QIODevice::WriteOnly)) {
        QTextStream out(&PID);
        out << getpid();
        PID.close();
    }

    a.loadTranslator();

    return a.exec();
}
