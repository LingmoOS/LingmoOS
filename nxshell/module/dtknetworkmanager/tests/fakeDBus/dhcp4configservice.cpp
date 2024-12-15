// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhcp4configservice.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QDBusMetaType>

FakeDHCP4ConfigService::FakeDHCP4ConfigService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

FakeDHCP4ConfigService::~FakeDHCP4ConfigService()
{
    unregisterService();
}

void FakeDHCP4ConfigService::registerService()
{
    qDBusRegisterMetaType<Config>();
    qRegisterMetaType<Config>("Config");
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

void FakeDHCP4ConfigService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(Path);
    bus.unregisterService(Service);
}
