// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notificationmanagerservice.h"
#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>

FakeNotificationManagerService::FakeNotificationManagerService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

FakeNotificationManagerService::~FakeNotificationManagerService()
{
    unregisterService();
}

void FakeNotificationManagerService::registerService()
{
    const QString &Service = QLatin1String("com.deepin.dde.FakeNotification");
    const QString &Path = QLatin1String("/com/deepin/dde/FakeNotification");
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(Service)) {
        QString errorMsg = bus.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";
        qWarning() << QString("Can't register the %1 service, %2.").arg(Service).arg(errorMsg);
    }
    if (!bus.registerObject(Path, this, QDBusConnection::ExportScriptableContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(Path);
    }
}

void FakeNotificationManagerService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject("/com/deepin/dde/FakeNotification");
    bus.unregisterService("com.deepin.dde.FakeNotification");
}
