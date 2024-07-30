/*
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device_p.h"
#include "manager.h"
#include "manager_p.h"
#include "teamdevice_p.h"

NetworkManager::TeamDevicePrivate::TeamDevicePrivate(const QString &path, TeamDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::TeamDevicePrivate::~TeamDevicePrivate()
{
}

NetworkManager::TeamDevice::TeamDevice(const QString &path, QObject *parent)
    : Device(*new TeamDevicePrivate(path, this), parent)
{
    Q_D(TeamDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::TeamDevice::~TeamDevice()
{
}

NetworkManager::Device::Type NetworkManager::TeamDevice::type() const
{
    return NetworkManager::Device::Team;
}

bool NetworkManager::TeamDevice::carrier() const
{
    Q_D(const TeamDevice);

    return d->carrier;
}

QString NetworkManager::TeamDevice::hwAddress() const
{
    Q_D(const TeamDevice);

    return d->hwAddress;
}

QStringList NetworkManager::TeamDevice::slaves() const
{
    Q_D(const TeamDevice);
    return d->slaves;
}

QString NetworkManager::TeamDevice::config() const
{
    Q_D(const TeamDevice);
    return d->config;
}

void NetworkManager::TeamDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(TeamDevice);

    if (property == QLatin1String("Carrier")) {
        carrier = value.toBool();
        Q_EMIT q->carrierChanged(carrier);
    } else if (property == QLatin1String("HwAddress")) {
        hwAddress = value.toString();
        Q_EMIT q->hwAddressChanged(hwAddress);
    } else if (property == QLatin1String("Slaves")) {
        QStringList list;
        const QList<QDBusObjectPath> opList = qdbus_cast<QList<QDBusObjectPath>>(value);
        for (const QDBusObjectPath &op : opList) {
            list << op.path();
        }
        slaves = list;
        Q_EMIT q->slavesChanged(slaves);
    } else if (property == QLatin1String("Config")) {
        config = value.toString();
        Q_EMIT q->configChanged(config);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_teamdevice.cpp"
#include "moc_teamdevice_p.cpp"
