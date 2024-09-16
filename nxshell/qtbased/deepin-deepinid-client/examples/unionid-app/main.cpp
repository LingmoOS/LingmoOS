// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>

#include <app.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService("com.deepin.deepinid.App")) {
        qDebug() << "register service failed" << sessionBus.lastError();
        return -1;
    }

    App app;
    if (!sessionBus.registerObject("/com/deepin/deepinid/App",
                                   &app,
                                   QDBusConnection::ExportScriptableSlots)) {
        qDebug() << "register object failed" << sessionBus.lastError();
        return -2;
    }

    return a.exec();
}
