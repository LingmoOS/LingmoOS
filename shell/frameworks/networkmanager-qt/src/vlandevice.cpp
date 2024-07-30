/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device_p.h"
#include "manager.h"
#include "vlandevice_p.h"

NetworkManager::VlanDevicePrivate::VlanDevicePrivate(const QString &path, VlanDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , carrier(false)
{
}

NetworkManager::VlanDevice::~VlanDevice()
{
}

NetworkManager::VlanDevice::VlanDevice(const QString &path, QObject *parent)
    : Device(*new VlanDevicePrivate(path, this), parent)
{
    Q_D(VlanDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::VlanDevicePrivate::~VlanDevicePrivate()
{
}

NetworkManager::Device::Type NetworkManager::VlanDevice::type() const
{
    return NetworkManager::Device::Vlan;
}

bool NetworkManager::VlanDevice::carrier() const
{
    Q_D(const VlanDevice);

    return d->carrier;
}

QString NetworkManager::VlanDevice::hwAddress() const
{
    Q_D(const VlanDevice);

    return d->hwAddress;
}

NetworkManager::Device::Ptr NetworkManager::VlanDevice::parent() const
{
    if (NetworkManager::checkVersion(1, 0, 0)) {
        Q_D(const VlanDevice);

        return NetworkManager::findNetworkInterface(d->parent);
    } else {
        return NetworkManager::Device::Ptr(nullptr);
    }
}

uint NetworkManager::VlanDevice::vlanId() const
{
    Q_D(const VlanDevice);

    return d->vlanId;
}

void NetworkManager::VlanDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(VlanDevice);

    if (property == QLatin1String("Carrier")) {
        carrier = value.toBool();
        Q_EMIT q->carrierChanged(carrier);
    } else if (property == QLatin1String("HwAddress")) {
        hwAddress = value.toString();
        Q_EMIT q->hwAddressChanged(hwAddress);
    } else if (property == QLatin1String("Parent")) {
        parent = value.value<QDBusObjectPath>().path();
        Q_EMIT q->parentChanged(parent);
    } else if (property == QLatin1String("VlanId")) {
        vlanId = value.toUInt();
        Q_EMIT q->vlanIdChanged(vlanId);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_vlandevice.cpp"
#include "moc_vlandevice_p.cpp"
