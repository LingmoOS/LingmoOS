/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "adsldevice_p.h"

NetworkManager::AdslDevicePrivate::AdslDevicePrivate(const QString &path, AdslDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , carrier(false)
{
}

NetworkManager::AdslDevice::~AdslDevice()
{
}

NetworkManager::AdslDevice::AdslDevice(const QString &path, QObject *parent)
    : Device(*new AdslDevicePrivate(path, this), parent)
{
    Q_D(AdslDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::AdslDevicePrivate::~AdslDevicePrivate()
{
}

NetworkManager::Device::Type NetworkManager::AdslDevice::type() const
{
    return NetworkManager::Device::Adsl;
}

bool NetworkManager::AdslDevice::carrier() const
{
    Q_D(const AdslDevice);

    return d->carrier;
}

void NetworkManager::AdslDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(AdslDevice);

    if (property == QLatin1String("Carrier")) {
        carrier = value.toBool();
        Q_EMIT q->carrierChanged(carrier);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_adsldevice.cpp"
#include "moc_adsldevice_p.cpp"
