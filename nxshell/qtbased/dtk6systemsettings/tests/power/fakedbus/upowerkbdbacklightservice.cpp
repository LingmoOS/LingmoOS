// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "upowerkbdbacklightservice.h"

#include <qdbusconnection.h>

UPowerKbdBacklightService::UPowerKbdBacklightService(QObject *parent)
    : QObject(parent)
    , m_reset(false)
{
    registerService();
}

UPowerKbdBacklightService::~UPowerKbdBacklightService()
{
    unRegisterService();
}

void UPowerKbdBacklightService::registerService()
{
    const QString &service = QLatin1String("com.deepin.daemon.FakePower");
    const QString &path = QLatin1String("/com/deepin/daemon/FakePower");
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(service)) {
        QString errorMsg = bus.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
    }
    if (!bus.registerObject(path, this, QDBusConnection::ExportScriptableContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
    }
}

void UPowerKbdBacklightService::unRegisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QLatin1String("/com/deepin/daemon/FakePower"));
    bus.unregisterService(QLatin1String("com.deepin.daemon.FakePower"));
}

// pubilc slots

qint32 UPowerKbdBacklightService::GetBrightness() const
{
    return 0;
}

qint32 UPowerKbdBacklightService::GetMaxBrightness() const
{
    return 255;
}

void UPowerKbdBacklightService::SetBrightness(const qint32 value)
{
    Q_UNUSED(value)
    m_reset = true;
}
