/*
    SPDX-FileCopyrightText: 2011 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2011 Will Stephenson <wstephenson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bluetoothdevice.h"
#include "bluetoothdevice_p.h"
#include "manager_p.h"

#include "nmdebug.h"

NetworkManager::BluetoothDevicePrivate::BluetoothDevicePrivate(const QString &path, BluetoothDevice *q)
    : ModemDevicePrivate(path, q)
#ifdef NMQT_STATIC
    , btIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    , btIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::BluetoothDevice::BluetoothDevice(const QString &path, QObject *parent)
    : ModemDevice(*new BluetoothDevicePrivate(path, this), parent)
{
    Q_D(BluetoothDevice);

    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(d->btIface.staticInterfaceName(), path);
    if (!initialProperties.isEmpty()) {
        d->propertiesChanged(initialProperties);
    }

}

NetworkManager::BluetoothDevice::~BluetoothDevice()
{
}

NetworkManager::Device::Type NetworkManager::BluetoothDevice::type() const
{
    return NetworkManager::Device::Bluetooth;
}

void NetworkManager::BluetoothDevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(BluetoothDevice);

    if (property == QLatin1String("Name")) {
        name = value.toString();
        Q_EMIT q->nameChanged(name);
    } else if (property == QLatin1String("HwAddress")) {
        hardwareAddress = value.toString();
    } else if (property == QLatin1String("BtCapabilities")) {
        btCapabilities = static_cast<NetworkManager::BluetoothDevice::Capabilities>(value.toUInt());
    } else {
        DevicePrivate::propertyChanged(property, value);
    }
}

NetworkManager::BluetoothDevice::Capabilities NetworkManager::BluetoothDevice::bluetoothCapabilities() const
{
    Q_D(const BluetoothDevice);
    return d->btCapabilities;
}

QString NetworkManager::BluetoothDevice::hardwareAddress() const
{
    Q_D(const BluetoothDevice);
    return d->hardwareAddress;
}

QString NetworkManager::BluetoothDevice::name() const
{
    Q_D(const BluetoothDevice);
    return d->name;
}

#include "moc_bluetoothdevice.cpp"
#include "moc_bluetoothdevice_p.cpp"
