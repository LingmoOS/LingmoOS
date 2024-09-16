// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hotspotcontrollernm.h"
#include "wirelessdevice.h"

#include <QDebug>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/WirelessSetting>

using namespace dde::network;

HotspotController_NM::HotspotController_NM(QObject *parent)
    : HotspotController(parent)
{
}

HotspotController_NM::~HotspotController_NM()
{
}

QSharedPointer<NetworkManager::WirelessDevice> HotspotController_NM::findWirelessDevice(WirelessDevice *wirelessDevice) const
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->type() == NetworkManager::Device::Type::Wifi && device->uni() == wirelessDevice->path())
            return device.staticCast<NetworkManager::WirelessDevice>();
    }

    return NetworkManager::WirelessDevice::Ptr();
}

HotspotItem *HotspotController_NM::addConnection(WirelessDevice *device, NetworkManager::Connection::Ptr connection)
{
    // 只过滤无线连接
    if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless)
        return nullptr;

    // 获取无线连接的设置
    NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
            .dynamicCast<NetworkManager::WirelessSetting>();
    if (wirelessSetting.isNull())
        return nullptr;

    // 只有mode为Ap的连接才是自建的热点
    if (wirelessSetting->mode() != NetworkManager::WirelessSetting::NetworkMode::Ap)
        return nullptr;

    qCDebug(DNC) << QString("new Hotspoint connection appeared, device: %1 %2, connection name:%3").arg(device->path()).arg(device->interface()).arg(connection->name());
    auto createJson = [ wirelessSetting ](NetworkManager::Connection::Ptr connection) {
        if (connection.isNull())
            return QJsonObject();

        QJsonObject json;
        json.insert("Path", connection->path());
        json.insert("Uuid", connection->uuid());
        json.insert("Id", connection->settings()->id());
        json.insert("IfcName", connection->settings()->interfaceName());
        NetworkManager::WirelessSetting::Ptr wsSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                .dynamicCast<NetworkManager::WirelessSetting>();
        if (wsSetting.isNull())
            wsSetting = wirelessSetting;
        json.insert("HwAddress", QString(wsSetting->macAddress()));
        json.insert("ClonedAddress", QString(wsSetting->clonedMacAddress()));
        json.insert("Ssid", QString(wsSetting->ssid()));
        json.insert("Hidden", false);

        return json;
    };

    // 判断当前连接是否在列表中
    if (m_deviceHotpots.contains(device)) {
        const QList<HotspotItem *> &hotspotItems = m_deviceHotpots[device];
        for (HotspotItem *item : hotspotItems) {
            if (item->connection()->path() != connection->path())
                continue;

            item->setConnection(createJson(connection));
            return nullptr;
        }
    }

    // 新建连接
    HotspotItem *hotspotItem = new HotspotItem(device);
    // 当连接信息发生变化的时候，例如连接名称变化，需要更新
    connect(connection.data(), &NetworkManager::Connection::updated, hotspotItem, [ this, device, hotspotItem, createJson, connection ] {
        hotspotItem->setConnection(createJson(connection));
        QMap<WirelessDevice *, QList<HotspotItem *>> itemInfo;
        itemInfo[device] << hotspotItem;
        Q_EMIT itemChanged(itemInfo);
    });

    hotspotItem->setConnection(createJson(connection));
    hotspotItem->m_devicePath = device->path();
    hotspotItem->m_device = device;
    hotspotItem->updateTimeStamp(connection->settings()->timestamp());
    m_deviceHotpots[device] << hotspotItem;

    return hotspotItem;
}

void HotspotController_NM::onActiveConnectionChanged(WirelessDevice *device, NetworkManager::ActiveConnection::Ptr activeConnection)
{
    if (!m_deviceHotpots.contains(device))
        return;

    // 如果当前连接不为空，则判断当前的活动连接是否为热点，如果不为热点，则不做任何操作
    if (!activeConnection.isNull()) {
        if (activeConnection->connection()->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless)
            return;
        // 获取无线连接的设置
        NetworkManager::WirelessSetting::Ptr wirelessSetting = activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                .dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting.isNull())
            return;

        // 只有mode为Ap的连接才是自建的热点
        if (wirelessSetting->mode() != NetworkManager::WirelessSetting::NetworkMode::Ap)
            return;
    }

    QList<HotspotItem *> items = m_deviceHotpots[device];
    // 如果当前的活动连接为空，就让每个连接都设置为断开状态
    for (HotspotItem *item : items) {
        item->setConnectionStatus(ConnectionStatus::Deactivated);
        item->setActiveConnection(QString());
    }
    if (!activeConnection.isNull()) {
        QList<HotspotItem *>::iterator itItem = std::find_if(items.begin(), items.end(), [ activeConnection ](HotspotItem *item) {
             return item->connection()->path() == activeConnection->connection()->path();
        });
        if (itItem == items.end())
            return;

        HotspotItem *hotspotItem = *itItem;
        ConnectionStatus currentState = convertStateFromNetworkManager(activeConnection->state());
        hotspotItem->setConnectionStatus(currentState);
        if (currentState == ConnectionStatus::Activated) {
            hotspotItem->setActiveConnection(activeConnection->path());
            hotspotItem->updateTimeStamp(activeConnection->connection()->settings()->timestamp());
        }
        connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, this, [ this, activeConnection ](NetworkManager::ActiveConnection::State state) {
            WirelessDevice *device = nullptr;
            HotspotItem *currentHotspotItem = nullptr;
            for (auto it = m_deviceHotpots.begin(); it != m_deviceHotpots.end(); it++) {
                for (HotspotItem *item : it.value()) {
                    if (item->connection()->path() != activeConnection->connection()->path())
                        continue;

                    device = it.key();
                    currentHotspotItem = item;
                    break;
                }
            }
            if (!device || !currentHotspotItem)
                return;

            ConnectionStatus newState = convertStateFromNetworkManager(state);
            if ((currentHotspotItem->status() == ConnectionStatus::Activating && newState == ConnectionStatus::Activated)
                    || (currentHotspotItem->status() == ConnectionStatus::Deactivating && newState == ConnectionStatus::Deactivated)) {
                Q_EMIT enableHotspotSwitch(true);
            }

            currentHotspotItem->setConnectionStatus(newState);
            if (newState == ConnectionStatus::Activated) {
                currentHotspotItem->setActiveConnection(activeConnection->path());
                // 更新连接时间为当前的时间
                activeConnection->connection()->settings()->setTimestamp(QDateTime::currentDateTime());
                currentHotspotItem->updateTimeStamp(activeConnection->connection()->settings()->timestamp());
            }

            qCDebug(DNC) << "active connection changed, connection path:" << activeConnection->path() << ", state:" << newState;
            Q_EMIT activeConnectionChanged({ device });
        });
    }

    Q_EMIT activeConnectionChanged( { device } );
}

void HotspotController_NM::sortItem()
{
    for (auto it = m_deviceHotpots.begin(); it != m_deviceHotpots.end(); it++) {
        // 便利每个连接的热点进行排序
        QList<HotspotItem *> &items = it.value();
        std::sort(items.begin(), items.end(), [](HotspotItem *item1, HotspotItem *item2) { return item1->connection()->ssid() < item2->connection()->ssid(); });
    }
}

void HotspotController_NM::onConnectionAdded(const QString &connectionUni)
{
    NetworkManager::WirelessDevice *wirelessDevice = qobject_cast<NetworkManager::WirelessDevice *>(sender());
    if (!wirelessDevice)
        return;

    NetworkManager::Connection::List availabelConnections = wirelessDevice->availableConnections();
    NetworkManager::Connection::List::iterator newConnection = std::find_if(availabelConnections.begin(), availabelConnections.end(), [ connectionUni ](NetworkManager::Connection::Ptr connection) {
        if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless)
            return false;

        // 获取无线连接的设置
        NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                .dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting.isNull())
            return false;

        // 只有mode为Ap的连接才是自建的热点
        if (wirelessSetting->mode() != NetworkManager::WirelessSetting::NetworkMode::Ap)
            return false;

        return connection->path() == connectionUni;
    });

    if (newConnection == availabelConnections.end())
        return;

    QList<WirelessDevice *>::iterator itDevice = std::find_if(m_devices.begin(), m_devices.end(), [ wirelessDevice ](WirelessDevice *device) { return device->path() == wirelessDevice->uni(); });
    if (itDevice == m_devices.end())
        return;

    qCDebug(DNC) << "connection added:" << connectionUni;
    WirelessDevice *device = *itDevice;
    HotspotItem *hotspotItem = addConnection(device, *newConnection);
    if (hotspotItem) {
        QMap<WirelessDevice *, QList<HotspotItem *>> hotspotItems;
        hotspotItems[device] << hotspotItem;
        emit itemAdded(hotspotItems);
    }
}

void HotspotController_NM::onConnectionRemoved(const QString &connectionUni)
{
    NetworkManager::WirelessDevice *wirelessDevice = qobject_cast<NetworkManager::WirelessDevice *>(sender());
    if (!wirelessDevice)
        return;

    // 查找对应的NetworkDevice
    QList<WirelessDevice *>::iterator itDevice = std::find_if(m_devices.begin(), m_devices.end(), [ wirelessDevice ](WirelessDevice *device) { return wirelessDevice->uni() == device->path(); });
    if (itDevice == m_devices.end())
        return;

    qCDebug(DNC) << QString("removed hotspotItem device: %1,%2, hotspot path:%3").arg(wirelessDevice->interfaceName()).arg(wirelessDevice->uni()).arg(connectionUni);
    WirelessDevice *device = *itDevice;
    if (!m_deviceHotpots.contains(device))
        return;

    QList<HotspotItem *> &hotspotItem = m_deviceHotpots[device];
    QList<HotspotItem *> rmItems;
    for (HotspotItem *item : hotspotItem) {
        if (item->connection()->path() != connectionUni)
            continue;

        qCDebug(DNC) << "hotspotItem name: " << item->name();
        rmItems << item;
    }

    if (rmItems.size() == 0)
        return;

    // 先从列表中移除消失的热点项
    for (HotspotItem *item : rmItems)
        hotspotItem.removeAll(item);

    // 用信号告诉外面已经消失的热点项
    QMap<WirelessDevice *, QList<HotspotItem *>> rmHotspotItems;
    rmHotspotItems[device] = rmItems;
    emit itemRemoved(rmHotspotItems);

    // 删除合适的热点项
    for (HotspotItem *item : rmItems)
        delete item;
}

void HotspotController_NM::setEnabled(WirelessDevice *device, const bool enable)
{
    qCInfo(DNC) << "hotspot enable status changed, device:" << device->interface() << "enabled:" << enable;
    // 如果是连接热点，则打开最后一次连接的热点
    if (enable) {
        // 如果当前的设备没有建立热点，无需做任何处理
        if (!m_deviceHotpots.contains(device))
            return;

        QList<HotspotItem *> items = m_deviceHotpots[device];
        std::sort(items.begin(), items.end(), [](HotspotItem *item1, HotspotItem *item2) {
            if (!item1->timeStamp().isValid() && !item2->timeStamp().isValid())
                return item1->connection()->id() > item2->connection()->id();
            if (!item1->timeStamp().isValid())
                return false;
            if (!item2->timeStamp().isValid())
                return true;

            return item1->timeStamp() > item2->timeStamp();
        });
        if (items.size() > 0) {
            NetworkManager::activateConnection(items.first()->connection()->path(), device->path(), "/");
        }
    } else {
        QSharedPointer<NetworkManager::WirelessDevice> wirelessDevice = findWirelessDevice(device);
        if (wirelessDevice.isNull())
            return;

        NetworkManager::ActiveConnection::Ptr activeConnection = wirelessDevice->activeConnection();
        if (activeConnection.isNull())
            return;

        NetworkManager::deactivateConnection(activeConnection->path());
    }
}

bool HotspotController_NM::enabled(WirelessDevice *device)
{
    return device->hotspotEnabled();
}

bool HotspotController_NM::supportHotspot()
{
    return m_devices.size() > 0;
}

void HotspotController_NM::connectItem(HotspotItem *item)
{
    WirelessDevice *device = nullptr;
    for (auto it = m_deviceHotpots.begin(); it != m_deviceHotpots.end(); it++) {
        QList<HotspotItem *> items = it.value();
        if (items.contains(item)) {
            device = it.key();
            break;
        }
    }

    if (!device)
        return;

    qCInfo(DNC) << "connect hotspot item:" << item->name() << "device:" << device->path();
    device->setEnabled(true);
    NetworkManager::activateConnection(item->connection()->path(), device->path(), "/");
}

void HotspotController_NM::connectItem(WirelessDevice *device, const QString &uuid)
{
    if (!m_deviceHotpots.contains(device))
        return;

    QList<HotspotItem *> items = m_deviceHotpots[device];
    QList<HotspotItem *>::iterator itItem = std::find_if(items.begin(), items.end(), [ uuid ](HotspotItem *item) { return item->connection()->uuid() == uuid; });
    if (itItem == items.end())
        return;

    HotspotItem *item = *itItem;
    device->setEnabled(true);
    NetworkManager::activateConnection(item->connection()->path(), device->path(), "/");
}

void HotspotController_NM::disconnectItem(WirelessDevice *device)
{
    QSharedPointer<NetworkManager::WirelessDevice> wirelessDevice = findWirelessDevice(device);
    if (wirelessDevice.isNull())
        return;

    NetworkManager::ActiveConnection::Ptr activeConnection = wirelessDevice->activeConnection();
    if (!activeConnection.isNull())
        NetworkManager::deactivateConnection(activeConnection->path());
}

QList<HotspotItem *> HotspotController_NM::items(WirelessDevice *device)
{
    return m_deviceHotpots.value(device);
}

QList<WirelessDevice *> HotspotController_NM::devices()
{
    return m_devices;
}

void HotspotController_NM::updateDevices(const QList<NetworkDeviceBase *> &devices)
{
    bool oldEnabled = (m_devices.size() > 0);
    QList<WirelessDevice *> newDevices;
    QList<WirelessDevice *> wirelessDevices;
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() != DeviceType::Wireless || !device->supportHotspot())
            continue;

        WirelessDevice *wirelessDevice = static_cast<WirelessDevice *>(device);
        wirelessDevices << wirelessDevice;
        if (!m_devices.contains(wirelessDevice))
            newDevices << wirelessDevice;
    }

    // 对比之前的设备信息和新的设备信息，对新增的设备设置相关信号的连接并创建item
    QList<WirelessDevice *> rmDevices;
    for (WirelessDevice *device : m_devices) {
        if (wirelessDevices.contains(device))
            continue;

        rmDevices << device;
    }

    m_devices = wirelessDevices;
    QMap<WirelessDevice *, QList<HotspotItem *>> rmDeviceItems;
    // 从列表中删除不存在的设备
    for (WirelessDevice *device : rmDevices) {
        // 删除热点项
        QList<HotspotItem *> items = m_deviceHotpots.value(device);
        m_deviceHotpots.remove(device);
        if (items.size() > 0)
            rmDeviceItems[device] << items;
    }

    QMap<WirelessDevice *, QList<HotspotItem *>> newHotspotItems;
    // 添加新的设备到列表中
    for (WirelessDevice *device : newDevices) {
        // 新增每个热点项
        NetworkManager::WirelessDevice::Ptr wirelessDevice = findWirelessDevice(device);
        if (wirelessDevice.isNull())
            continue;

        // 监听设备的活动连接的变化的信号
        connect(wirelessDevice.data(), &NetworkManager::WirelessDevice::activeConnectionChanged, device, [ device, wirelessDevice, this ] {
            if (wirelessDevice) {
                onActiveConnectionChanged(device, wirelessDevice->activeConnection());
            }
        });

        // 监听设备连接发生变化的信号
        connect(wirelessDevice.data(), &NetworkManager::WirelessDevice::availableConnectionAppeared, this, &HotspotController_NM::onConnectionAdded, Qt::UniqueConnection);
        connect(wirelessDevice.data(), &NetworkManager::WirelessDevice::availableConnectionDisappeared, this, &HotspotController_NM::onConnectionRemoved, Qt::UniqueConnection);
        // 先查找出所有的热点的连接
        NetworkManager::Connection::List connections = wirelessDevice->availableConnections();
        for (NetworkManager::Connection::Ptr connection : connections) {
            HotspotItem *hotspotItem = addConnection(device, connection);
            if (hotspotItem)
                newHotspotItems[device] << hotspotItem;
        }

        onActiveConnectionChanged(device, wirelessDevice->activeConnection());
    }

    sortItem();

    // 通知外部是否可用
    bool hotspotEnabled = (m_devices.size() > 0);
    if (oldEnabled != hotspotEnabled) {
        Q_EMIT enabledChanged(hotspotEnabled);
    }

    // 通知外部新增了设备
    if (newDevices.size() > 0) {
        Q_EMIT deviceAdded(newDevices);
    }

    // 通知外部新增了热点
    if (newHotspotItems.size() > 0)
        emit itemAdded(newHotspotItems);

    if (rmDevices.size() > 0) {
        // 通知外部移除了热点
        if (rmDeviceItems.size() > 0)
            Q_EMIT itemRemoved(rmDeviceItems);

        // 通知外部移除了设备
        Q_EMIT deviceRemove(rmDevices);

        QList<QList<HotspotItem *>> rmItems = rmDeviceItems.values();
        for (QList<HotspotItem *> &items : rmItems) {
            for (HotspotItem *item : items)
                delete item;
        }
    }
}
