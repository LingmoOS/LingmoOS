/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device_p.h"
#include "infinibanddevice_p.h"
#include "manager.h"

NetworkManager::InfinibandDevicePrivate::InfinibandDevicePrivate(const QString &path, InfinibandDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , carrier(false)
{
}

NetworkManager::InfinibandDevice::~InfinibandDevice()
{
}

NetworkManager::InfinibandDevice::InfinibandDevice(const QString &path, QObject *parent)
    : Device(*new InfinibandDevicePrivate(path, this), parent)
{
    Q_D(InfinibandDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::InfinibandDevicePrivate::~InfinibandDevicePrivate()
{
}

NetworkManager::Device::Type NetworkManager::InfinibandDevice::type() const
{
    return NetworkManager::Device::InfiniBand;
}

bool NetworkManager::InfinibandDevice::carrier() const
{
    Q_D(const InfinibandDevice);

    return d->carrier;
}

QString NetworkManager::InfinibandDevice::hwAddress() const
{
    Q_D(const InfinibandDevice);

    return d->hwAddress;
}

void NetworkManager::InfinibandDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(InfinibandDevice);

    if (property == QLatin1String("Carrier")) {
        carrier = value.toBool();
        Q_EMIT q->carrierChanged(carrier);
    } else if (property == QLatin1String("HwAddress")) {
        hwAddress = value.toString();
        Q_EMIT q->hwAddressChanged(hwAddress);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_infinibanddevice.cpp"
#include "moc_infinibanddevice_p.cpp"
