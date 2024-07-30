/*
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device_p.h"
#include "gredevice_p.h"
#include "manager.h"

NetworkManager::GreDevicePrivate::GreDevicePrivate(const QString &path, GreDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::GreDevicePrivate::~GreDevicePrivate()
{
}

NetworkManager::GreDevice::GreDevice(const QString &path, QObject *parent)
    : Device(*new GreDevicePrivate(path, this), parent)
{
    Q_D(GreDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }

}

NetworkManager::GreDevice::~GreDevice()
{
}

NetworkManager::Device::Type NetworkManager::GreDevice::type() const
{
    return NetworkManager::Device::Gre;
}

ushort NetworkManager::GreDevice::inputFlags() const
{
    Q_D(const GreDevice);
    return d->inputFlags;
}

ushort NetworkManager::GreDevice::outputFlags() const
{
    Q_D(const GreDevice);
    return d->outputFlags;
}

uint NetworkManager::GreDevice::inputKey() const
{
    Q_D(const GreDevice);
    return d->inputKey;
}

uint NetworkManager::GreDevice::outputKey() const
{
    Q_D(const GreDevice);
    return d->outputKey;
}

QString NetworkManager::GreDevice::localEnd() const
{
    Q_D(const GreDevice);
    return d->localEnd;
}

QString NetworkManager::GreDevice::remoteEnd() const
{
    Q_D(const GreDevice);
    return d->remoteEnd;
}

QString NetworkManager::GreDevice::parent() const
{
    Q_D(const GreDevice);
    return d->parent;
}

bool NetworkManager::GreDevice::pathMtuDiscovery() const
{
    Q_D(const GreDevice);
    return d->pathMtuDiscovery;
}

uchar NetworkManager::GreDevice::tos() const
{
    Q_D(const GreDevice);
    return d->tos;
}

uchar NetworkManager::GreDevice::ttl() const
{
    Q_D(const GreDevice);
    return d->ttl;
}

void NetworkManager::GreDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(GreDevice);

    if (property == QLatin1String("InputFlags")) {
        inputFlags = static_cast<ushort>(value.toUInt());
        Q_EMIT q->inputFlagsChanged(inputFlags);
    } else if (property == QLatin1String("OutputFlags")) {
        outputFlags = static_cast<ushort>(value.toUInt());
        Q_EMIT q->outputFlagsChanged(outputFlags);
    } else if (property == QLatin1String("InputKey")) {
        inputKey = value.toUInt();
        Q_EMIT q->inputKeyChanged(inputKey);
    } else if (property == QLatin1String("OutputKey")) {
        outputKey = value.toUInt();
        Q_EMIT q->outputKeyChanged(outputKey);
    } else if (property == QLatin1String("Local")) {
        localEnd = value.toString();
        Q_EMIT q->localEndChanged(localEnd);
    } else if (property == QLatin1String("Remote")) {
        remoteEnd = value.toString();
        Q_EMIT q->remoteEndChanged(remoteEnd);
    } else if (property == QLatin1String("Parent")) {
        parent = value.toString();
        Q_EMIT q->parentChanged(parent);
    } else if (property == QLatin1String("PathMtuDiscovery")) {
        pathMtuDiscovery = value.toBool();
        Q_EMIT q->pathMtuDiscoveryChanged(pathMtuDiscovery);
    } else if (property == QLatin1String("Tos")) {
        tos = static_cast<uchar>(value.toUInt());
        Q_EMIT q->tosChanged(tos);
    } else if (property == QLatin1String("Ttl")) {
        ttl = static_cast<uchar>(value.toUInt());
        Q_EMIT q->ttlChanged(ttl);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_gredevice.cpp"
#include "moc_gredevice_p.cpp"
