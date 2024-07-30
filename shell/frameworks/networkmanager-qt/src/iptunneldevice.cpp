/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device_p.h"
#include "iptunneldevice_p.h"
#include "manager.h"

NetworkManager::IpTunnelDevicePrivate::IpTunnelDevicePrivate(const QString &path, IpTunnelDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::IpTunnelDevicePrivate::~IpTunnelDevicePrivate()
{
}

NetworkManager::IpTunnelDevice::IpTunnelDevice(const QString &path, QObject *parent)
    : Device(*new IpTunnelDevicePrivate(path, this), parent)
{
    Q_D(IpTunnelDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }

}

NetworkManager::IpTunnelDevice::~IpTunnelDevice()
{
}

NetworkManager::Device::Type NetworkManager::IpTunnelDevice::type() const
{
    return NetworkManager::Device::IpTunnel;
}

uchar NetworkManager::IpTunnelDevice::encapsulationLimit() const
{
    Q_D(const IpTunnelDevice);
    return d->encapsulationLimit;
}

uint NetworkManager::IpTunnelDevice::flowLabel() const
{
    Q_D(const IpTunnelDevice);
    return d->flowLabel;
}

QString NetworkManager::IpTunnelDevice::inputKey() const
{
    Q_D(const IpTunnelDevice);
    return d->inputKey;
}

QString NetworkManager::IpTunnelDevice::local() const
{
    Q_D(const IpTunnelDevice);
    return d->local;
}

uint NetworkManager::IpTunnelDevice::mode() const
{
    Q_D(const IpTunnelDevice);
    return d->mode;
}

QString NetworkManager::IpTunnelDevice::outputKey() const
{
    Q_D(const IpTunnelDevice);
    return d->outputKey;
}

NetworkManager::Device::Ptr NetworkManager::IpTunnelDevice::parent() const
{
    Q_D(const IpTunnelDevice);
    return NetworkManager::findNetworkInterface(d->parent);
}

bool NetworkManager::IpTunnelDevice::pathMtuDiscovery() const
{
    Q_D(const IpTunnelDevice);
    return d->pathMtuDiscovery;
}

QString NetworkManager::IpTunnelDevice::remote() const
{
    Q_D(const IpTunnelDevice);
    return d->remote;
}

uchar NetworkManager::IpTunnelDevice::tos() const
{
    Q_D(const IpTunnelDevice);
    return d->tos;
}

uchar NetworkManager::IpTunnelDevice::ttl() const
{
    Q_D(const IpTunnelDevice);
    return d->ttl;
}

void NetworkManager::IpTunnelDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(IpTunnelDevice);

    if (property == QLatin1String("EncapsulationLimit")) {
        encapsulationLimit = static_cast<ushort>(value.toUInt());
        Q_EMIT q->encapsulationLimitChanged(encapsulationLimit);
    } else if (property == QLatin1String("FlowLabel")) {
        flowLabel = value.toUInt();
        Q_EMIT q->flowLabelChanged(flowLabel);
    } else if (property == QLatin1String("InputKey")) {
        inputKey = value.toString();
        Q_EMIT q->inputKeyChanged(inputKey);
    } else if (property == QLatin1String("Local")) {
        local = value.toString();
        Q_EMIT q->localChanged(local);
    } else if (property == QLatin1String("Mode")) {
        mode = value.toUInt();
        Q_EMIT q->modeChanged(mode);
    } else if (property == QLatin1String("OutputKey")) {
        outputKey = value.toString();
        Q_EMIT q->outputKeyChanged(outputKey);
    } else if (property == QLatin1String("Parent")) {
        parent = value.toString();
        Q_EMIT q->parentChanged(parent);
    } else if (property == QLatin1String("PathMtuDiscovery")) {
        pathMtuDiscovery = value.toBool();
        Q_EMIT q->pathMtuDiscoveryChanged(pathMtuDiscovery);
    } else if (property == QLatin1String("Remote")) {
        remote = value.toString();
        Q_EMIT q->remoteChanged(remote);
    } else if (property == QLatin1String("Tos")) {
        tos = static_cast<ushort>(value.toUInt());
        Q_EMIT q->tosChanged(tos);
    } else if (property == QLatin1String("Ttl")) {
        ttl = static_cast<ushort>(value.toUInt());
        Q_EMIT q->ttlChanged(ttl);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_iptunneldevice.cpp"
#include "moc_iptunneldevice_p.cpp"
