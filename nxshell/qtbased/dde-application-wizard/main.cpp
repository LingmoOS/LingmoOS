// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QDBusConnection>
#include <QDebug>
#include <DGuiApplicationHelper>

#include "dbus/launcher1compat.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName(QStringLiteral("dde-application-wizard"));

    Dtk::Gui::DGuiApplicationHelper::loadTranslator();

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(QStringLiteral("org.deepin.dde.daemon.Launcher1")) ||
        !connection.registerObject(QStringLiteral("/org/deepin/dde/daemon/Launcher1"), &Launcher1Compat::instance())) {
        qFatal("register dbus service failed");
    }
  
    return app.exec();
}
