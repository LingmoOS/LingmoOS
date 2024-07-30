/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wirelessdevice.h"

#include <QDBusConnection>

WirelessDevice::WirelessDevice(QObject *parent)
    : Device(parent)
    , m_activeAccessPoint(QDBusObjectPath("/"))
    , m_bitrate(0)
    , m_mode(2)
    , m_wirelessCapabilities(0)
    , m_accessPointCounter(0)
{
}

WirelessDevice::~WirelessDevice()
{
    for (auto it = m_accessPoints.cbegin(); it != m_accessPoints.cend(); ++it) {
        const QDBusObjectPath &ap = it.key();
        QDBusConnection::sessionBus().unregisterObject(ap.path());
        Q_EMIT AccessPointRemoved(ap);
    }

    qDeleteAll(m_accessPoints);

    QVariantMap map;
    map.insert(QLatin1String("AvailableConnections"), QVariant::fromValue<QList<QDBusObjectPath>>(QList<QDBusObjectPath>()));
    Q_EMIT PropertiesChanged(map);
}

QList<QDBusObjectPath> WirelessDevice::accessPoints() const
{
    return m_accessPoints.keys();
}

QDBusObjectPath WirelessDevice::activeAccessPoint() const
{
    return m_activeAccessPoint;
}

uint WirelessDevice::bitrate() const
{
    return m_bitrate;
}

QString WirelessDevice::hwAddress() const
{
    return m_hwAddress;
}

uint WirelessDevice::mode() const
{
    return m_mode;
}

QString WirelessDevice::permHwAddress() const
{
    return m_permHwAddress;
}

uint WirelessDevice::wirelessCapabilities() const
{
    return m_wirelessCapabilities;
}

void WirelessDevice::addAccessPoint(AccessPoint *accessPoint)
{
    QString newApPath = QString("/org/kde/fakenetwork/AccessPoints/") + QString::number(m_accessPointCounter++);
    accessPoint->setAccessPointPath(newApPath);
    m_accessPoints.insert(QDBusObjectPath(newApPath), accessPoint);
    QDBusConnection::sessionBus().registerObject(newApPath, accessPoint, QDBusConnection::ExportScriptableContents);

    Q_EMIT AccessPointAdded(QDBusObjectPath(newApPath));
}

void WirelessDevice::removeAccessPoint(AccessPoint *accessPoint)
{
    m_accessPoints.remove(QDBusObjectPath(accessPoint->accessPointPath()));

    Q_EMIT AccessPointRemoved(QDBusObjectPath(accessPoint->accessPointPath()));
}

void WirelessDevice::setActiveAccessPoint(const QString &activeAccessPoint)
{
    m_activeAccessPoint = QDBusObjectPath(activeAccessPoint);
}

void WirelessDevice::setBitrate(uint bitrate)
{
    m_bitrate = bitrate;
}

void WirelessDevice::setHwAddress(const QString &hwAddress)
{
    m_hwAddress = hwAddress;
}

void WirelessDevice::setMode(uint mode)
{
    m_mode = mode;
}

void WirelessDevice::setPermHwAddress(const QString &permHwAddress)
{
    m_permHwAddress = permHwAddress;
}

void WirelessDevice::setState(uint state)
{
    Device::setState(state);

    // TODO: set speed, etc.
}

void WirelessDevice::setWirelessCapabilities(uint capabilities)
{
    m_wirelessCapabilities = capabilities;
}

QList<QDBusObjectPath> WirelessDevice::GetAccessPoints()
{
    return m_accessPoints.keys();
}

QList<QDBusObjectPath> WirelessDevice::GetAllAccessPoints()
{
    return m_accessPoints.keys();
}

void WirelessDevice::RequestScan(const QVariantMap &options)
{
    Q_UNUSED(options);
}

#include "moc_wirelessdevice.cpp"
