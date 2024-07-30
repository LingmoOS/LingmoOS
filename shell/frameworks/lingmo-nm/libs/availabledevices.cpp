/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "availabledevices.h"

#include <NetworkManagerQt/Manager>

AvailableDevices::AvailableDevices(QObject *parent)
    : QObject(parent)
{
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Modem) {
            m_modemDeviceAvailable = true;
        } else if (device->type() == NetworkManager::Device::Wifi) {
            m_wirelessDeviceAvailable = true;
        } else if (device->type() == NetworkManager::Device::Ethernet) {
            m_wiredDeviceAvailable = true;
        } else if (device->type() == NetworkManager::Device::Bluetooth) {
            m_bluetoothDeviceAvailable = true;
        }
    }

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &AvailableDevices::deviceAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &AvailableDevices::deviceRemoved);
}

AvailableDevices::~AvailableDevices() = default;

bool AvailableDevices::isWiredDeviceAvailable() const
{
    return m_wiredDeviceAvailable;
}

bool AvailableDevices::isWirelessDeviceAvailable() const
{
    return m_wirelessDeviceAvailable;
}

bool AvailableDevices::isModemDeviceAvailable() const
{
    return m_modemDeviceAvailable;
}

bool AvailableDevices::isBluetoothDeviceAvailable() const
{
    return m_bluetoothDeviceAvailable;
}

void AvailableDevices::deviceAdded(const QString &dev)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(dev);

    if (device) {
        if (device->type() == NetworkManager::Device::Modem && !m_modemDeviceAvailable) {
            m_modemDeviceAvailable = true;
            Q_EMIT modemDeviceAvailableChanged(true);
        } else if (device->type() == NetworkManager::Device::Wifi && !m_wirelessDeviceAvailable) {
            m_wirelessDeviceAvailable = true;
            Q_EMIT wirelessDeviceAvailableChanged(true);
        } else if (device->type() == NetworkManager::Device::Ethernet && !m_wiredDeviceAvailable) {
            m_wiredDeviceAvailable = true;
            Q_EMIT wiredDeviceAvailableChanged(true);
        } else if (device->type() == NetworkManager::Device::Bluetooth && !m_bluetoothDeviceAvailable) {
            m_bluetoothDeviceAvailable = true;
            Q_EMIT bluetoothDeviceAvailableChanged(true);
        }
    }
}

void AvailableDevices::deviceRemoved()
{
    bool wired = false;
    bool wireless = false;
    bool modem = false;
    bool bluetooth = false;

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Modem) {
            modem = true;
        } else if (device->type() == NetworkManager::Device::Wifi) {
            wireless = true;
        } else if (device->type() == NetworkManager::Device::Ethernet) {
            wired = true;
        } else if (device->type() == NetworkManager::Device::Bluetooth) {
            bluetooth = true;
        }
    }

    if (!wired && m_wiredDeviceAvailable) {
        m_wiredDeviceAvailable = false;
        Q_EMIT wiredDeviceAvailableChanged(false);
    }

    if (!wireless && m_wirelessDeviceAvailable) {
        m_wirelessDeviceAvailable = false;
        Q_EMIT wirelessDeviceAvailableChanged(false);
    }

    if (!modem && m_modemDeviceAvailable) {
        m_modemDeviceAvailable = false;
        Q_EMIT modemDeviceAvailableChanged(false);
    }

    if (!bluetooth && m_bluetoothDeviceAvailable) {
        m_bluetoothDeviceAvailable = false;
        Q_EMIT bluetoothDeviceAvailableChanged(false);
    }
}

#include "moc_availabledevices.cpp"
