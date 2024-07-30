/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bridgedevice_p.h"
#include "manager_p.h"

NetworkManager::BridgeDevicePrivate::BridgeDevicePrivate(const QString &path, BridgeDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , carrier(false)
{
}

NetworkManager::BridgeDevicePrivate::~BridgeDevicePrivate()
{
}

NetworkManager::BridgeDevice::BridgeDevice(const QString &path, QObject *parent)
    : Device(*new BridgeDevicePrivate(path, this), parent)
{
    Q_D(BridgeDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }
}

NetworkManager::BridgeDevice::~BridgeDevice()
{
}

NetworkManager::Device::Type NetworkManager::BridgeDevice::type() const
{
    return NetworkManager::Device::Bridge;
}

bool NetworkManager::BridgeDevice::carrier() const
{
    Q_D(const BridgeDevice);

    return d->carrier;
}

QString NetworkManager::BridgeDevice::hwAddress() const
{
    Q_D(const BridgeDevice);

    return d->hwAddress;
}

QStringList NetworkManager::BridgeDevice::slaves() const
{
    Q_D(const BridgeDevice);

    return d->slaves;
}

void NetworkManager::BridgeDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(BridgeDevice);

    if (property == QLatin1String("Carrier")) {
        carrier = value.toBool();
        Q_EMIT q->carrierChanged(carrier);
    } else if (property == QLatin1String("HwAddress")) {
        hwAddress = value.toString();
        Q_EMIT q->hwAddressChanged(hwAddress);
    } else if (property == QLatin1String("Slaves")) {
        QStringList list;
        const QList<QDBusObjectPath> opList = qdbus_cast<QList<QDBusObjectPath>>(value);
        for (const QDBusObjectPath &op : opList) {
            list << op.path();
        }
        slaves = list;
        Q_EMIT q->slavesChanged(slaves);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_bridgedevice.cpp"
