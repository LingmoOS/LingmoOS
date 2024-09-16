// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountsmanagerservice.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

FakeAccountsManagerService::FakeAccountsManagerService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

FakeAccountsManagerService::~FakeAccountsManagerService()
{
    unregisterService();
}

void FakeAccountsManagerService::registerService()
{
    const QString &Service = QLatin1String("com.deepin.daemon.FakeAccounts");
    const QString &Path = QLatin1String("/com/deepin/daemon/FakeAccounts");
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

void FakeAccountsManagerService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject("/com/deepin/daemon/FakeAccounts");
    bus.unregisterService("com.deepin.daemon.FakeAccounts");
}
