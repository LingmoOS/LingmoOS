/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wireddevice.h"

WiredDevice::WiredDevice(QObject *parent)
    : Device(parent)
{
}

WiredDevice::~WiredDevice()
{
    QVariantMap map;
    map.insert(QLatin1String("AvailableConnections"), QVariant::fromValue<QList<QDBusObjectPath>>(QList<QDBusObjectPath>()));
    Q_EMIT PropertiesChanged(map);
}

bool WiredDevice::carrier() const
{
    return m_carrier;
}

QString WiredDevice::hwAddress() const
{
    return m_hwAddress;
}

QString WiredDevice::permHwAddress() const
{
    return m_permHwAddress;
}

uint WiredDevice::speed() const
{
    return m_speed;
}

void WiredDevice::setCarrier(bool carrier)
{
    m_carrier = carrier;
}

void WiredDevice::setHwAddress(const QString &hwAddress)
{
    m_hwAddress = hwAddress;
}

void WiredDevice::setPermanentHwAddress(const QString &permanentHwAddress)
{
    m_permHwAddress = permanentHwAddress;
}

void WiredDevice::setSpeed(uint speed)
{
    m_speed = speed;
}

void WiredDevice::setState(uint state)
{
    Device::setState(state);
    // set speed
}

#include "moc_wireddevice.cpp"
