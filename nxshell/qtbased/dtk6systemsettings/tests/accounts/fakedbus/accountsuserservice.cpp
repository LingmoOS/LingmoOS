// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountsuserservice.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

FakeAccountsUserService::FakeAccountsUserService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

FakeAccountsUserService::~FakeAccountsUserService()
{
    unregisterService();
}

void FakeAccountsUserService::registerService()
{
    registerType();
    const QString &Service = QLatin1String("com.deepin.daemon.FakeAccounts");
    const QString &Path = QLatin1String("/com/deepin/daemon/FakeAccounts/User1000");
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

void FakeAccountsUserService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject("/com/deepin/daemon/FakeAccounts/User1000");
    bus.unregisterService("com.deepin.daemon.FakeAccounts");
}
