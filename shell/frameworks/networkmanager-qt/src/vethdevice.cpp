/*
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "manager.h"
#include "manager_p.h"
#include "vethdevice_p.h"

NetworkManager::VethDevicePrivate::VethDevicePrivate(const QString &path, VethDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::VethDevicePrivate::~VethDevicePrivate()
{
}

NetworkManager::VethDevice::VethDevice(const QString &path, QObject *parent)
    : Device(*new VethDevicePrivate(path, this), parent)
{
    Q_D(VethDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::VethDevice::~VethDevice()
{
}

NetworkManager::Device::Type NetworkManager::VethDevice::type() const
{
    return NetworkManager::Device::Veth;
}

QString NetworkManager::VethDevice::peer() const
{
    Q_D(const VethDevice);
    return d->peer;
}

void NetworkManager::VethDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(VethDevice);

    if (property == QLatin1String("Peer")) {
        peer = value.toString();
        Q_EMIT q->peerChanged(peer);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_vethdevice.cpp"
#include "moc_vethdevice_p.cpp"
