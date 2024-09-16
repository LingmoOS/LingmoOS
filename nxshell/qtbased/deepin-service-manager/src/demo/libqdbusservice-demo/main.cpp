// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "demoadaptor.h"
#include "service.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Service s;
    DemoAdaptor adp(&s);
    // 从QDBusService对象拿到 QDBusConnection 防止注册对象不一致，导致无法正常管理权限
    QDBusConnection connection = s.qDbusConnection();
    if (!connection.registerObject("/org/deepin/service/sdk/demo", &s)) {
        qWarning() << "failed to register dbus object" << connection.lastError().message();
    }
    if (!connection.registerService("org.deepin.service.sdk.demo")) {
        qWarning() << "failed to register dbus object" << connection.lastError().message();
    }

    QObject::connect(&s, &Service::dbusLog, [](const QString &log) {
        qDebug() << log;
    });

    return a.exec();
}
