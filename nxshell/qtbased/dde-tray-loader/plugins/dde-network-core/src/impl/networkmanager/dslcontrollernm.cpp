// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslcontrollernm.h"
#include "wireddevice.h"
#include "netutils.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/PppoeSetting>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/Settings>

using namespace dde::network;

DSLController_NM::DSLController_NM(QObject *parent)
    : DSLController(parent)
{
    initMember();
    initConnection();
}

DSLController_NM::~DSLController_NM()
{
}

void DSLController_NM::initMember()
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->type() != NetworkManager::Device::Type::Ethernet
                || !device->managed()
                || !(device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP))
            continue;

        m_devices << device.staticCast<NetworkManager::WiredDevice>();
    }

    // 先更新设备自己的连接
    for (NetworkManager::WiredDevice::Ptr wiredDevice : m_devices) {
        NetworkManager::Connection::List connections = getConnections(wiredDevice);
        for (NetworkManager::Connection::Ptr connection : connections) {
            addPppoeConnection(wiredDevice, connection);
        }

        updateActiveConnectionInfo(wiredDevice.data());
    }
}

NetworkManager::Connection::List DSLController_NM::getConnections(const NetworkManager::WiredDevice::Ptr &device)
{
    NetworkManager::Connection::List connections = NetworkManager::listConnections();
    NetworkManager::Connection::List deviceConnections = device->availableConnections();
    QStringList devicePaths;
    for (NetworkManager::Connection::Ptr deviceConnection : deviceConnections) {
        devicePaths << deviceConnection->path();
    }

    for (NetworkManager::Connection::Ptr connection : connections) {
        const QString &interfaceName = connection->settings()->interfaceName();
        if (!interfaceName.isEmpty() && interfaceName != device->interfaceName())
            continue;

        if (!devicePaths.contains(connection->path()))
            deviceConnections << connection;
    }

    return deviceConnections;
}

void DSLController_NM::initConnection()
{
    // 响应设备变化的信号
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &DSLController_NM::onDeviceAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &DSLController_NM::onDeviceRemoved);
    for (NetworkManager::WiredDevice::Ptr wiredDevice : m_devices) {
        initDeviceConnection(wiredDevice);
    }
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionAdded, this, [ this ](const QString &path) {
        for (NetworkManager::WiredDevice::Ptr wiredDevice : m_devices)
            connectionAppeared(wiredDevice, NetworkManager::listConnections(), path);
    });
}

void DSLController_NM::initDeviceConnection(const NetworkManager::WiredDevice::Ptr &wiredDevice)
{
    connect(wiredDevice.data(), &NetworkManager::WiredDevice::availableConnectionAppeared, this, [ wiredDevice, this ](const QString &uni) {
        connectionAppeared(wiredDevice, wiredDevice->availableConnections(), uni);
    });

    connect(wiredDevice.data(), &NetworkManager::WiredDevice::availableConnectionDisappeared, this, [ wiredDevice, this](const QString &uni) {
        QList<DSLItem *>::iterator itItem = std::find_if(m_dslItems.begin(), m_dslItems.end(), [ uni ](DSLItem *item) {
            return (item->connection()->path() == uni);
        });

        if (itItem != m_dslItems.end()) {
            DSLItem *rmItem = *itItem;
            m_dslItems.removeOne(rmItem);
            Q_EMIT itemRemoved({rmItem});
            delete rmItem;
        }
    });

    // 无线网卡不管是否down，都显示，因为在开启飞行模式后，需要显示网卡的信息
    auto deviceCreateOrRemove = [ this ](const NetworkManager::WiredDevice::Ptr &device) {
        if (device->managed() && (device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP)) {
            if (!m_devices.contains(device)) {
                m_devices << device;
                initDeviceConnection(device);
                NetworkManager::Connection::List connections = getConnections(device);
                for (NetworkManager::Connection::Ptr connection : connections) {
                     addPppoeConnection(device, connection);
                }

                updateActiveConnectionInfo(device.data());
            }
        } else {
            if (m_devices.contains(device))
                m_devices.removeOne(device);
        }
    };

    connect(wiredDevice.get(), &NetworkManager::Device::interfaceFlagsChanged, this, [ wiredDevice, deviceCreateOrRemove ] {
        deviceCreateOrRemove(wiredDevice);
    });
    connect(wiredDevice.get(), &NetworkManager::Device::managedChanged, this, [ wiredDevice, deviceCreateOrRemove ] {
        deviceCreateOrRemove(wiredDevice);
    });

    connect(wiredDevice.data(), &NetworkManager::WiredDevice::activeConnectionChanged, this, &DSLController_NM::onActiveConnectionChanged, Qt::UniqueConnection);
}

void DSLController_NM::connectionAppeared(const NetworkManager::WiredDevice::Ptr &wiredDevice, NetworkManager::Connection::List connections, const QString &uni)
{
    NetworkManager::Connection::List::iterator itConnection =
            std::find_if(connections.begin(), connections.end(), [ uni ](const NetworkManager::Connection::Ptr &connection) {
        return (connection->path() == uni);
    });

    if (itConnection != connections.end()) {
        DSLItem *dslItem = addPppoeConnection(wiredDevice, *itConnection);
        if (dslItem)
            Q_EMIT itemAdded({dslItem});
    }
}

DSLItem *DSLController_NM::addPppoeConnection(NetworkManager::WiredDevice::Ptr device, NetworkManager::Connection::Ptr connection)
{
    // 此处只添加pppoe的连接
    if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Pppoe)
        return nullptr;

    QList<DSLItem *>::iterator itdsl = std::find_if(m_dslItems.begin(), m_dslItems.end(), [ = ](DSLItem *item) {
        return (item->connection()->path() == connection->path());
    });

    if (itdsl != m_dslItems.end()) {
        // 如果查找到了对应的连接，则返回即可,无需继续,这里为了处理多次调用该函数的情况
        return nullptr;
    }

    // 如果没有找到对应的连接，则新增连接
    auto itemJson = [ device ](NetworkManager::Connection::Ptr connection){
        QJsonObject json;
        json.insert("Path", connection->path());
        json.insert("Uuid", connection->uuid());
        json.insert("Id", connection->settings()->id());
        json.insert("IfcName", connection->settings()->interfaceName());
        NetworkManager::WiredSetting::Ptr wiredSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wired).staticCast<NetworkManager::WiredSetting>();
        QString macAddress = device->permanentHardwareAddress();
        macAddress.remove(":");
        if (macAddress.toUpper() == wiredSetting->macAddress().toHex().toUpper())
            json.insert("HwAddress", device->permanentHardwareAddress());
        else
            json.insert("HwAddress", QString());

        json.insert("ClonedAddress", QString(wiredSetting->clonedMacAddress()));
        json.insert("Ssid", QString());
        json.insert("Hidden", false);
        return json;
    };

    // 如果从列表中没有找到，新增一条DSL连接
    DSLItem *item = new DSLItem();
    item->setConnection(itemJson(connection));
    item->updateTimeStamp(connection->settings()->timestamp());
    m_dslItems << item;

    // 当连接的名称发送变化的时候，需要更新界面的显示
    connect(connection.data(), &NetworkManager::Connection::updated, item, [ this, item, itemJson, connection ] {
        item->setConnection(itemJson(connection));
        item->updateTimeStamp(connection->settings()->timestamp());
        Q_EMIT itemChanged({ item });
    });

    // 按照ID进行排序
    std::sort(m_dslItems.begin(), m_dslItems.end(), [](DSLItem *item1, DSLItem *item2) {
        return item1->connection()->id() < item2->connection()->id();
    });

    return item;
}

void DSLController_NM::updateActiveConnectionInfo(NetworkManager::WiredDevice *wiredDevice)
{
    const NetworkManager::ActiveConnection::Ptr activeConnection = wiredDevice->activeConnection();
    if (activeConnection.isNull() || activeConnection->connection()->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Pppoe)
        return;

    // 将当前设备的所有连接状态变为未连接
    for (DSLItem *item : m_dslItems) {
        item->setConnectionStatus(ConnectionStatus::Deactivated);
        item->setActiveConnection(QString());
    }

    // 查找当前对应状态的DSL的项，改变其连接状态
    QList<DSLItem *>::iterator itDsl = std::find_if(m_dslItems.begin(), m_dslItems.end(), [ wiredDevice, activeConnection ](DSLItem *item) {
        return item->connection()->path() == activeConnection->connection()->path();
    });

    if (itDsl == m_dslItems.end())
        return;

    DSLItem *dslItem = *itDsl;
    // 当状态发生变化的时候，更新连接状态，同时发出信号通知界面当前状态发生了变化
    auto updateActiveConnection = [ this, dslItem, activeConnection ](NetworkManager::ActiveConnection::State state) {
        ConnectionStatus status = convertStateFromNetworkManager(state);
        dslItem->setConnectionStatus(status);
        if (status == ConnectionStatus::Activated) {
            activeConnection->connection()->settings()->setTimestamp(QDateTime::currentDateTime());
            dslItem->updateTimeStamp(activeConnection->connection()->settings()->timestamp());
            dslItem->setActiveConnection(activeConnection->path());
        }
        qCDebug(DNC) << "actit connection changed" << state;
        Q_EMIT activeConnectionChanged();
    };

    connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, this, updateActiveConnection, Qt::UniqueConnection);
    updateActiveConnection(activeConnection->state());
}

void DSLController_NM::onActiveConnectionChanged()
{
    NetworkManager::WiredDevice *wiredDevice = qobject_cast<NetworkManager::WiredDevice *>(sender());
    if (!wiredDevice)
        return;

    updateActiveConnectionInfo(wiredDevice);
}

void DSLController_NM::onDeviceAdded(const QString &uni)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
    if (device.isNull() || !device->managed() || !(device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP))
        return;

    qCInfo(DNC) << "new device:" << uni;
    NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
    m_devices << wiredDevice;
    initDeviceConnection(wiredDevice);
    NetworkManager::Connection::List connections = getConnections(wiredDevice);

    for (NetworkManager::Connection::Ptr connection : connections) {
        addPppoeConnection(wiredDevice, connection);
    }

    updateActiveConnectionInfo(wiredDevice.data());
}

void DSLController_NM::onDeviceRemoved(const QString &uni)
{
    qCInfo(DNC) << "device removed:" << uni;
    QList<NetworkManager::WiredDevice::Ptr> rmDevices;
    for (NetworkManager::WiredDevice::Ptr device : m_devices) {
        if (device->uni() == uni)
            rmDevices << device;
    }

    for (NetworkManager::WiredDevice::Ptr device : rmDevices)
        m_devices.removeAll(device);
}

void DSLController_NM::connectItem(DSLItem *item)
{
    if (!m_dslItems.contains(item))
        return;

    QString devicePath;
    for (NetworkManager::WiredDevice::Ptr wiredDevice : m_devices) {
        NetworkManager::Connection::List connections = wiredDevice->availableConnections();
        NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [ item ](NetworkManager::Connection::Ptr conn) {
            return conn->path() == item->connection()->path();
        });
        if (itConnection != connections.end()) {
            devicePath = wiredDevice->uni();
            break;
        }
    }

    qCInfo(DNC) << "connect pppoe item" << item->connection()->path() << ", device" << devicePath;

    if (!devicePath.isEmpty())
        NetworkManager::activateConnection(item->connection()->path(), devicePath, QString());
}

void DSLController_NM::connectItem(const QString &uuid)
{
    QList<DSLItem *>::iterator itDsl = std::find_if(m_dslItems.begin(), m_dslItems.end(), [ uuid ](DSLItem *item) {
        return item->connection()->uuid() == uuid;
    });

    if (itDsl != m_dslItems.end())
        connectItem(*itDsl);
}

void DSLController_NM::disconnectItem(WiredDevice *device)
{
    NetworkManager::WiredDevice::List::iterator itDevice = std::find_if(m_devices.begin(), m_devices.end(), [ device ](NetworkManager::WiredDevice::Ptr wiredDevice) { return wiredDevice->uni() == device->path(); });
    if (itDevice == m_devices.end())
        return;

    NetworkManager::ActiveConnection::Ptr activeConnection = (*itDevice)->activeConnection();
    if (activeConnection->connection()->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Pppoe)
        return;

    qCInfo(DNC) << "disconnect pppoe item" << activeConnection->path() << "device" << device->interface();
    NetworkManager::deactivateConnection(activeConnection->path());
}

QList<DSLItem *> DSLController_NM::items() const
{
    return m_dslItems;
}
