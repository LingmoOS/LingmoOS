/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "hwaddrcombobox.h"

#include <NetworkManagerQt/BluetoothDevice>
#include <NetworkManagerQt/BondDevice>
#include <NetworkManagerQt/BridgeDevice>
#include <NetworkManagerQt/InfinibandDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/OlpcMeshDevice>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VlanDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>

HwAddrComboBox::HwAddrComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);

    connect(this, &HwAddrComboBox::editTextChanged, this, &HwAddrComboBox::slotEditTextChanged);
    connect(this, QOverload<int>::of(&HwAddrComboBox::currentIndexChanged), this, &HwAddrComboBox::slotCurrentIndexChanged);
}

bool HwAddrComboBox::isValid() const
{
    if (hwAddress().isEmpty()) {
        return true;
    }

    return NetworkManager::macAddressIsValid(hwAddress());
}

QString HwAddrComboBox::hwAddress() const
{
    QString result;
    if (!m_dirty)
        result = itemData(currentIndex()).toString();
    else
        result = currentText();

    // qCDebug(PLASMA_NM_EDITOR_LOG)() << "Result:" << currentIndex() << result;

    return result;
}

void HwAddrComboBox::slotEditTextChanged(const QString &)
{
    m_dirty = true;
    Q_EMIT hwAddressChanged();
}

void HwAddrComboBox::slotCurrentIndexChanged(int)
{
    m_dirty = false;
    Q_EMIT hwAddressChanged();
}

void HwAddrComboBox::init(NetworkManager::Device::Type deviceType, const QString &address)
{
    m_initialAddress = address;

    // qCDebug(PLASMA_NM_EDITOR_LOG) << "Initial address:" << m_initialAddress;

    QString deviceName;
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        const NetworkManager::Device::Type type = device->type();
        if (type == deviceType) {
            if (address == hwAddressFromDevice(device).toString()) {
                if (device->state() == NetworkManager::Device::Activated) {
                    deviceName = device->ipInterfaceName();
                } else {
                    deviceName = device->interfaceName();
                }
            }
            addAddressToCombo(device);
        }
    }

    const int index = findData(m_initialAddress);
    if (index == -1) {
        if (!m_initialAddress.isEmpty()) {
            const QString text = QStringLiteral("%1 (%2)").arg(deviceName, m_initialAddress);
            insertItem(0, text, m_initialAddress);
        } else {
            insertItem(0, m_initialAddress, m_initialAddress);
        }
        setCurrentIndex(0);
    } else {
        setCurrentIndex(index);
    }
}

void HwAddrComboBox::addAddressToCombo(const NetworkManager::Device::Ptr &device)
{
    const QVariant data = hwAddressFromDevice(device);
    // qCDebug(PLASMA_NM_EDITOR_LOG) << "Data:" << data;

    QString name;
    if (device->state() == NetworkManager::Device::Activated)
        name = device->ipInterfaceName();
    else
        name = device->interfaceName();

    // qCDebug(PLASMA_NM_EDITOR_LOG) << "Name:" << name;

    if (!data.isNull()) {
        if (name == data.toString()) {
            addItem(data.toString(), data);
        } else {
            addItem(QStringLiteral("%1 (%2)").arg(name, data.toString()), data);
        }
    }
}

QVariant HwAddrComboBox::hwAddressFromDevice(const NetworkManager::Device::Ptr &device)
{
    const NetworkManager::Device::Type type = device->type();

    QVariant data;
    if (type == NetworkManager::Device::Ethernet) {
        data = device->as<NetworkManager::WiredDevice>()->permanentHardwareAddress();
    } else if (type == NetworkManager::Device::Wifi) {
        data = device->as<NetworkManager::WirelessDevice>()->permanentHardwareAddress();
    } else if (type == NetworkManager::Device::Bluetooth) {
        data = device->as<NetworkManager::BluetoothDevice>()->hardwareAddress();
    } else if (type == NetworkManager::Device::OlpcMesh) {
        data = device->as<NetworkManager::OlpcMeshDevice>()->hardwareAddress();
    } else if (type == NetworkManager::Device::InfiniBand) {
        data = device->as<NetworkManager::InfinibandDevice>()->hwAddress();
    } else if (type == NetworkManager::Device::Bond) {
        data = device->as<NetworkManager::BondDevice>()->hwAddress();
    } else if (type == NetworkManager::Device::Bridge) {
        data = device->as<NetworkManager::BridgeDevice>()->hwAddress();
    } else if (type == NetworkManager::Device::Vlan) {
        data = device->as<NetworkManager::VlanDevice>()->hwAddress();
    }

    return data;
}

#include "moc_hwaddrcombobox.cpp"
