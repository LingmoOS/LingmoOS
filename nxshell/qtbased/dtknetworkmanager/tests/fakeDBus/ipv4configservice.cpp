// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ipv4configservice.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QDBusMetaType>

FakeIPv4ConfigService::FakeIPv4ConfigService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

FakeIPv4ConfigService::~FakeIPv4ConfigService()
{
    unregisterService();
}

void FakeIPv4ConfigService::registerService()
{
    qDBusRegisterMetaType<Config>();
    qDBusRegisterMetaType<QList<Config>>();
    qRegisterMetaType<Config>("Config");
    qRegisterMetaType<QList<Config>>("QList<Config>");
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

void FakeIPv4ConfigService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(Path);
    bus.unregisterService(Service);
}
