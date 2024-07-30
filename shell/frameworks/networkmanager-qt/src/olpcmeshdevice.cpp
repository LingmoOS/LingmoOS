/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "olpcmeshdevice.h"
#include "manager_p.h"
#include "olpcmeshdevice_p.h"

#include "wimaxnsp.h"

NetworkManager::OlpcMeshDevicePrivate::OlpcMeshDevicePrivate(const QString &path, OlpcMeshDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::OlpcMeshDevice::OlpcMeshDevice(const QString &path, QObject *parent)
    : Device(*new OlpcMeshDevicePrivate(path, this), parent)
{
    Q_D(OlpcMeshDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::OlpcMeshDevice::~OlpcMeshDevice()
{
}

NetworkManager::Device::Type NetworkManager::OlpcMeshDevice::type() const
{
    return NetworkManager::Device::OlpcMesh;
}

QString NetworkManager::OlpcMeshDevice::hardwareAddress() const
{
    Q_D(const OlpcMeshDevice);
    return d->hardwareAddress;
}

uint NetworkManager::OlpcMeshDevice::activeChannel() const
{
    Q_D(const OlpcMeshDevice);
    return d->activeChannel;
}

NetworkManager::Device::Ptr NetworkManager::OlpcMeshDevice::companionDevice() const
{
    Q_D(const OlpcMeshDevice);
    return NetworkManager::findNetworkInterface(d->companion);
}

void NetworkManager::OlpcMeshDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(OlpcMeshDevice);

    if (property == QLatin1String("ActiveChannel")) {
        activeChannel = value.toUInt();
        Q_EMIT q->activeChannelChanged(activeChannel);
    } else if (property == QLatin1String("HwAddress")) {
        hardwareAddress = value.toString();
        Q_EMIT q->hardwareAddressChanged(hardwareAddress);
    } else if (property == QLatin1String("Companion")) {
        companion = qdbus_cast<QDBusObjectPath>(value).path();
        Q_EMIT q->companionChanged(NetworkManager::findNetworkInterface(companion));
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_olpcmeshdevice.cpp"
#include "moc_olpcmeshdevice_p.cpp"
