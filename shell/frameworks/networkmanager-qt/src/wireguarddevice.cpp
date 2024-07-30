/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wireguarddevice.h"
#include "manager_p.h"
#include "wireguarddevice_p.h"

NetworkManager::WireGuardDevicePrivate::WireGuardDevicePrivate(const QString &path, WireGuardDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , listenPort(0)
    , fwMark(0)
{
}

NetworkManager::WireGuardDevicePrivate::~WireGuardDevicePrivate()
{
}

NetworkManager::WireGuardDevice::WireGuardDevice(const QString &path, QObject *parent)
    : Device(*new WireGuardDevicePrivate(path, this), parent)
{
    Q_D(WireGuardDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::WireGuardDevice::~WireGuardDevice()
{
}

NetworkManager::Device::Type NetworkManager::WireGuardDevice::type() const
{
    return NetworkManager::Device::WireGuard;
}

QByteArray NetworkManager::WireGuardDevice::publicKey() const
{
    Q_D(const WireGuardDevice);

    return d->publicKey;
}

uint NetworkManager::WireGuardDevice::listenPort() const
{
    Q_D(const WireGuardDevice);

    return d->listenPort;
}

uint NetworkManager::WireGuardDevice::fwMark() const
{
    Q_D(const WireGuardDevice);

    return d->fwMark;
}

void NetworkManager::WireGuardDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(WireGuardDevice);

    if (property == QLatin1String("PublicKey")) {
        publicKey = value.toByteArray();
        Q_EMIT q->publicKeyChanged(publicKey);
    } else if (property == QLatin1String("ListenPort")) {
        listenPort = value.toUInt();
        Q_EMIT q->listenPortChanged(listenPort);
    } else if (property == QLatin1String("FwMark")) {
        fwMark = value.toUInt();
        Q_EMIT q->fwMarkChanged(fwMark);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_wireguarddevice.cpp"
