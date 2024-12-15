// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ipv6configservice.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QDBusMetaType>

FakeIPv6ConfigService::FakeIPv6ConfigService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

FakeIPv6ConfigService::~FakeIPv6ConfigService()
{
    unregisterService();
}

void FakeIPv6ConfigService::registerService()
{
    qDBusRegisterMetaType<QList<Config>>();
    qRegisterMetaType<QList<Config>>("QList<Config>");
    qDBusRegisterMetaType<QList<QByteArray>>();
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

void FakeIPv6ConfigService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(Path);
    bus.unregisterService(Service);
}
