/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wireddevice.h"
#include "manager.h"
#include "manager_p.h"
#include "wireddevice_p.h"

#include "nmdebug.h"

NetworkManager::WiredDevicePrivate::WiredDevicePrivate(const QString &path, WiredDevice *q)
    : DevicePrivate(path, q)
#ifdef NMQT_STATIC
    , wiredIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , wiredIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , bitrate(0)
    , carrier(false)
{
}

NetworkManager::WiredDevicePrivate::~WiredDevicePrivate()
{
}

NetworkManager::WiredDevice::WiredDevice(const QString &path, QObject *parent)
    : Device(*new NetworkManager::WiredDevicePrivate(path, this), parent)
{
    Q_D(WiredDevice);
#ifdef NMQT_STATIC
    connect(&d->wiredIface, &OrgFreedesktopNetworkManagerDeviceWiredInterface::PropertiesChanged, d, &WiredDevicePrivate::propertiesChanged);
#endif
    // Get all WiredDevices's properties at once
    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->wiredIface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }


}

NetworkManager::WiredDevice::~WiredDevice()
{
}

NetworkManager::Device::Type NetworkManager::WiredDevice::type() const
{
    return NetworkManager::Device::Ethernet;
}

QString NetworkManager::WiredDevice::hardwareAddress() const
{
    Q_D(const NetworkManager::WiredDevice);
    return d->hardwareAddress;
}

QString NetworkManager::WiredDevice::permanentHardwareAddress() const
{
    Q_D(const NetworkManager::WiredDevice);
    return d->permanentHardwareAddress;
}

int NetworkManager::WiredDevice::bitRate() const
{
    Q_D(const NetworkManager::WiredDevice);
    return d->bitrate;
}

bool NetworkManager::WiredDevice::carrier() const
{
    Q_D(const NetworkManager::WiredDevice);
    return d->carrier;
}

QStringList NetworkManager::WiredDevice::s390SubChannels() const
{
    Q_D(const NetworkManager::WiredDevice);
    return d->s390SubChannels;
}

void NetworkManager::WiredDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(NetworkManager::WiredDevice);

    if (property == QLatin1String("Carrier")) {
        carrier = value.toBool();
        Q_EMIT q->carrierChanged(carrier);
    } else if (property == QLatin1String("HwAddress")) {
        hardwareAddress = value.toString();
        Q_EMIT q->hardwareAddressChanged(hardwareAddress);
    } else if (property == QLatin1String("PermHwAddress")) {
        permanentHardwareAddress = value.toString();
        Q_EMIT q->permanentHardwareAddressChanged(permanentHardwareAddress);
    } else if (property == QLatin1String("Speed")) {
        bitrate = value.toUInt() * 1000;
        Q_EMIT q->bitRateChanged(bitrate);
    } else if (property == QLatin1String("S390Subchannels")) {
        s390SubChannels = value.toStringList();
        Q_EMIT q->s390SubChannelsChanged(s390SubChannels);
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

#include "moc_wireddevice.cpp"
#include "moc_wireddevice_p.cpp"
