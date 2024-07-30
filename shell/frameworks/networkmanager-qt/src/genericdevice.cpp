/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "genericdevice_p.h"
#include "manager.h"
#include "nmdebug.h"

NetworkManager::GenericDevicePrivate::GenericDevicePrivate(const QString &path, GenericDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::GenericDevicePrivate::~GenericDevicePrivate()
{
}

NetworkManager::GenericDevice::GenericDevice(const QString &path, QObject *parent)
    : Device(*new NetworkManager::GenericDevicePrivate(path, this), parent)
{
    Q_D(GenericDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::GenericDevice::~GenericDevice()
{
}

NetworkManager::Device::Type NetworkManager::GenericDevice::type() const
{
    return NetworkManager::Device::Generic;
}

QString NetworkManager::GenericDevice::hardwareAddress() const
{
    Q_D(const GenericDevice);
    return d->hwAddress;
}

QString NetworkManager::GenericDevice::typeDescription() const
{
    Q_D(const GenericDevice);
    return d->typeDescription;
}

void NetworkManager::GenericDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(GenericDevice);

    if (property == QLatin1String("HwAddress")) {
        hwAddress = value.toString();
        Q_EMIT q->hardwareAddressChanged(hwAddress);
    } else if (property == QLatin1String("TypeDescription")) {
        typeDescription = value.toString();
        Q_EMIT q->permanentHardwareAddressChanged(typeDescription);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_genericdevice.cpp"
#include "moc_genericdevice_p.cpp"
