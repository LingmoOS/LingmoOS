// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inputdevicesservice.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
InputDevicesService::InputDevicesService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_path(path)
{
    registerService(m_service, m_path);
}

InputDevicesService::~InputDevicesService()
{
    unregisterService();
}

bool InputDevicesService::registerService(const QString &service, const QString &path)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }
    return true;
}

void InputDevicesService::unregisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

quint32 InputDevicesService::WheelSpeed() const
{
    return m_wheelSpeed;
}

void InputDevicesService::SetWheelSpeed(quint32 speed)
{
    m_wheelSpeed = speed;
    Q_EMIT WheelSpeedChanged(speed);
}
DDEVICE_END_NAMESPACE
