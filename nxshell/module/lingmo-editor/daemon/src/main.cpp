// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus.h"
#include "dbusadaptor.h"

#include <QCoreApplication>
#include <QtDBus/QDBusConnection>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    DBus *dbus1 = new DBus;
    new DbusAdaptor(dbus1);

    QDBusConnection connection = QDBusConnection::systemBus();
    if (connection.isConnected()) {
        qDebug() << "Build lingmo-editor daemon success.";
    }

    if(!connection.registerService("com.lingmo.editor.daemon") ||
       !connection.registerObject("/", dbus1)){
        qDebug() << connection.lastError();

        app.exit(1);
        return 1;
    }

    return app.exec();
}
