/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device_p.h"
#include "macvlandevice_p.h"
#include "manager.h"

NetworkManager::MacVlanDevicePrivate::MacVlanDevicePrivate(const QString &path, MacVlanDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::MacVlanDevicePrivate::~MacVlanDevicePrivate()
{
}

NetworkManager::MacVlanDevice::MacVlanDevice(const QString &path, QObject *parent)
    : Device(*new MacVlanDevicePrivate(path, this), parent)
{
    Q_D(MacVlanDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::MacVlanDevice::~MacVlanDevice()
{
}

NetworkManager::Device::Type NetworkManager::MacVlanDevice::type() const
{
    return NetworkManager::Device::MacVlan;
}

QString NetworkManager::MacVlanDevice::mode() const
{
    Q_D(const MacVlanDevice);
    return d->mode;
}

bool NetworkManager::MacVlanDevice::noPromisc() const
{
    Q_D(const MacVlanDevice);
    return d->noPromisc;
}

QString NetworkManager::MacVlanDevice::parent() const
{
    Q_D(const MacVlanDevice);
    return d->parent;
}

void NetworkManager::MacVlanDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(MacVlanDevice);

    if (property == QLatin1String("Mode")) {
        mode = value.toString();
        Q_EMIT q->modeChanged(mode);
    } else if (property == QLatin1String("NoPromisc")) {
        noPromisc = value.toBool();
        Q_EMIT q->noPromiscChanged(noPromisc);
    } else if (property == QLatin1String("Parent")) {
        parent = value.toString();
        Q_EMIT q->parentChanged(parent);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_macvlandevice.cpp"
#include "moc_macvlandevice_p.cpp"
