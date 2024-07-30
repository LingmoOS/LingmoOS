/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bonddevice_p.h"
#include "device_p.h"
#include "manager.h"
#include "manager_p.h"

NetworkManager::BondDevicePrivate::BondDevicePrivate(const QString &path, BondDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , carrier(false)
{
}

NetworkManager::BondDevice::~BondDevice()
{
}

NetworkManager::BondDevice::BondDevice(const QString &path, QObject *parent)
    : Device(*new BondDevicePrivate(path, this), parent)
{
    Q_D(BondDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->iface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }

}

NetworkManager::BondDevicePrivate::~BondDevicePrivate()
{
}

NetworkManager::Device::Type NetworkManager::BondDevice::type() const
{
    return NetworkManager::Device::Bond;
}

bool NetworkManager::BondDevice::carrier() const
{
    Q_D(const BondDevice);

    return d->carrier;
}

QString NetworkManager::BondDevice::hwAddress() const
{
    Q_D(const BondDevice);

    return d->hwAddress;
}

QStringList NetworkManager::BondDevice::slaves() const
{
    Q_D(const BondDevice);

    return d->slaves;
}

void NetworkManager::BondDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(BondDevice);

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

#include "moc_bonddevice.cpp"
#include "moc_bonddevice_p.cpp"
