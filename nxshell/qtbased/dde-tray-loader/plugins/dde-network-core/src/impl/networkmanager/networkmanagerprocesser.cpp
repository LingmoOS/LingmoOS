// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicemanagerrealize.h"
#include "dslcontrollernm.h"
#include "hotspotcontrollernm.h"
#include "networkdevicebase.h"
#include "networkmanagerprocesser.h"
#include "proxycontrollernm.h"
#include "vpncontrollernm.h"
#include "networkdetails.h"
#include "netutils.h"
#include "objectmanager.h"
#include "networkdetailnmrealize.h"

#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/IpAddress>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/WirelessSetting>

#include <wireddevice.h>
#include <wirelessdevice.h>

using namespace dde::network;

const static QString NetworkManagerService = "org.freedesktop.NetworkManager";
const static QString NetworkManagerPath = "/org/freedesktop/NetworkManager";
const static QString NetworkManagerInterface = "org.freedesktop.NetworkManager";

NetworkManagerProcesser::NetworkManagerProcesser(bool sync, QObject *parent)
    : NetworkProcesser(parent)
    , ProcesserInterface()
    , m_proxyController(Q_NULLPTR)
    , m_vpnController(Q_NULLPTR)
    , m_dslController(Q_NULLPTR)
    , m_hotspotController(Q_NULLPTR)
    , m_connectivity(dde::network::Connectivity::Unknownconnectivity)
    , m_needDetails(false)
{
    initConnections();
    if (sync) {
        NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
        qCDebug(DNC) << "devices size:" << devices.size();
        for (NetworkManager::Device::Ptr device : devices) {
            qCDebug(DNC) << "device  interface:" << device->interfaceName() << "device uni:" << device->uni();
            onDevicesChanged({QDBusObjectPath(device->uni())});
        }
    } else {
        QDBusMessage getDevices = QDBusMessage::createMethodCall(NetworkManagerService, NetworkManagerPath, NetworkManagerInterface, "GetAllDevices");
        QDBusConnection::systemBus().callWithCallback(getDevices, this, SLOT(onDevicesChanged(QList<QDBusObjectPath>)));
    }
    QDBusMessage checkConnectivity = QDBusMessage::createMethodCall(NetworkManagerService, NetworkManagerPath, NetworkManagerInterface, "CheckConnectivity");
    QDBusConnection::systemBus().callWithCallback(checkConnectivity, this, SLOT(checkConnectivityFinished(quint32)));
}

NetworkManagerProcesser::~NetworkManagerProcesser()
{
}

void NetworkManagerProcesser::initConnections()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, [ this ](const QString &uni) {
        qCDebug(DNC) << "device added:" << uni;
        onDeviceAdded(uni);
    });
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, [ this ](const QString &uni) {
        qCDebug(DNC) << "device removed:" << uni;
        onDeviceRemove(uni);
        if (m_hotspotController) {
            m_hotspotController->updateDevices(m_devices);
        }
    });
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &NetworkManagerProcesser::onConnectivityChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionsChanged, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
    // 当网络状态发生变化时，也刷新Detail
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
}

void NetworkManagerProcesser::onDevicesChanged(const QList<QDBusObjectPath> &devices)
{
    for(QDBusObjectPath device : devices ) {
        qCDebug(DNC) << "Device added: " << device.path();
        onDeviceAdded(device.path());
    }
}

QList<NetworkDeviceBase *> NetworkManagerProcesser::devices()
{
    return m_devices;
}

dde::network::Connectivity NetworkManagerProcesser::connectivity()
{
    return m_connectivity;
}

QList<NetworkDetails *> NetworkManagerProcesser::networkDetails()
{
    if (!m_needDetails) {
        m_needDetails = true;
        onUpdateNetworkDetail();
    }

    return ObjectManager::instance()->networkDetails();
}

ProxyController *NetworkManagerProcesser::proxyController()
{
    if (!m_proxyController)
        m_proxyController = new ProxyController_NM(this);

    return m_proxyController;
}

VPNController *NetworkManagerProcesser::vpnController()
{
    if (!m_vpnController) {
        m_vpnController = new VPNController_NM(this);
        connect(m_vpnController, &VPNController_NM::activeConnectionChanged, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
    }

    return m_vpnController;
}

DSLController *NetworkManagerProcesser::dslController()
{
    if (!m_dslController) {
        m_dslController = new DSLController_NM(this);
        connect(m_dslController, &DSLController_NM::activeConnectionChanged, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
    }

    return m_dslController;
}

HotspotController *NetworkManagerProcesser::hotspotController()
{
    if (!m_hotspotController) {
        m_hotspotController = new HotspotController_NM(this);
        m_hotspotController->updateDevices(m_devices);
    }

    return m_hotspotController;
}

void NetworkManagerProcesser::deviceEnabledChanged()
{
    if (m_hotspotController)
        m_hotspotController->updateDevices(m_devices);
}

void NetworkManagerProcesser::sortDevice()
{
    auto getPathIndex = [](const QString path)->int {
        int index = path.lastIndexOf("/");
        QString tmpIndexValue = path.mid(index + 1);
        return tmpIndexValue.toInt();
    };
    // 有线网络始终在无线网络的前面，如果两者都是有线或者无线网络，则按照path的顺序来排序
    qSort(m_devices.begin(), m_devices.end(),  [ = ](NetworkDeviceBase *device1, NetworkDeviceBase *device2) {
        if (device1->deviceType() == DeviceType::Wired && device2->deviceType() == DeviceType::Wireless)
            return true;

        if (device1->deviceType() == DeviceType::Wireless && device2->deviceType() == DeviceType::Wired)
            return false;

        return getPathIndex(device1->path()) < getPathIndex(device2->path());
    });
}

void NetworkManagerProcesser::onUpdateNetworkDetail()
{
    if (!m_needDetails)
        return;

    ObjectManager *creator = ObjectManager::instance();
    creator->cleanupNetworkDetails();

    QMap<QString, NetworkManager::Device::Ptr> avaibleDevices;
    for (dde::network::NetworkDeviceBase *device: m_devices) {
        if (!device->isEnabled() || !device->available())
            continue;

        NetworkManager::Device::Ptr dev = NetworkManager::findNetworkInterface(device->path());
        if (dev)
            avaibleDevices[device->path()] = dev;
    }

    NetworkManager::ActiveConnection::List activeConns = NetworkManager::activeConnections();
    for (NetworkManager::ActiveConnection::Ptr activeConn : activeConns) {
        if (activeConn->state() != NetworkManager::ActiveConnection::State::Activated)
            continue;

        // 获取当前连接的所有的设备的信息
        QStringList devicePaths = activeConn->devices();
        for (const QString &devicePath : devicePaths) {
            if (!avaibleDevices.contains(devicePath))
                continue;
            // 遍历每个设备的活动连接
            creator->createNetworkDetail(new NetworkDetailNMRealize(avaibleDevices.value(devicePath), activeConn));
        }
    }

    Q_EMIT activeConnectionChange();
}

void NetworkManagerProcesser::onDeviceAdded(const QString &uni)
{
    auto deviceExist = [ this ](const QString &uni)->bool {
        for (NetworkDeviceBase *device : m_devices) {
            if (device->path() == uni)
                return true;
        }

        return false;
    };

    if (deviceExist(uni))
        return;

    NetworkManager::Device::Ptr currentDevice = NetworkManager::findNetworkInterface(uni);
    if (currentDevice.isNull() ||
            (currentDevice->type() != NetworkManager::Device::Wifi && currentDevice->type() != NetworkManager::Device::Ethernet))
        return;

    auto updateHotspot = [ this ] {
        if (m_hotspotController) {
            m_hotspotController->updateDevices(m_devices);
        }
    };

    auto createDevice = [ = ](const NetworkManager::Device::Ptr &device)->NetworkDeviceBase *{
        if (device->type() == NetworkManager::Device::Wifi) {
            // 无线网络
            NetworkManager::WirelessDevice::Ptr wDevice = device.staticCast<NetworkManager::WirelessDevice>();
            WirelessDeviceManagerRealize *deviceRealize = new WirelessDeviceManagerRealize(wDevice);
            deviceRealize->addProcesser(this);
            return new WirelessDevice(deviceRealize, Q_NULLPTR);
        }

        if (device->type() == NetworkManager::Device::Ethernet) {
            // 有线网络
            NetworkManager::WiredDevice::Ptr wDevice = device.staticCast<NetworkManager::WiredDevice>();
            DeviceManagerRealize *deviceRealize = new WiredDeviceManagerRealize(wDevice);
            return new WiredDevice(deviceRealize, Q_NULLPTR);
        }

        return nullptr;
    };

    // 无线网卡不管是否down，都显示，因为在开启飞行模式后，需要显示网卡的信息
    auto deviceCreateOrRemove = [ this, deviceExist, createDevice, updateHotspot ](const NetworkManager::Device::Ptr &device) {
        if (device->managed()
#ifdef USE_DEEPIN_NMQT
                    && ((device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP) ||
                    device->type() == NetworkManager::Device::Wifi)
#endif
                ) {
            // 如果由非manager变成manager的模式，则新增设备
            if (!deviceExist(device->uni())) {
                NetworkDeviceBase *newDevice = createDevice(device);
                if (newDevice) {
                    m_devices << newDevice;
                    sortDevice();
                    updateDeviceName();
                    onUpdateNetworkDetail();
                    Q_EMIT deviceAdded({ newDevice });
                    updateHotspot();
                }
            }
        } else {
            // 如果由manager变成非manager模式，则移除设备
            NetworkDeviceBase *rmDevice = nullptr;
            for (NetworkDeviceBase *dev : m_devices) {
                if (dev->path() == device->uni()) {
                    m_devices.removeOne(dev);
                    rmDevice = dev;
                    break;
                }
            }
            if (rmDevice) {
                Q_EMIT rmDevice->removed();
                sortDevice();
                updateDeviceName();
                onUpdateNetworkDetail();
                Q_EMIT deviceRemoved({ rmDevice });
                rmDevice->deleteLater();
                rmDevice = nullptr;
                updateHotspot();
            }
        }
    };

    if (!currentDevice->managed() || currentDevice->interfaceFlags() == 0) {
        // TODO: 临时解决方案，适用于ARM平台，从根本上解决需要从NetworkManagerQt入手
        // 在ARM下存在当前设备的manager为false，interfaceFlags为0，但是该设备实际是manager为true并且interfaceflags不为0,
        // 在这种情况下，根据实际情况创建设备
        switch (currentDevice->type()) {
        case NetworkManager::Device::Wifi: {
            currentDevice.reset(new NetworkManager::WirelessDevice(uni));
            m_deviceList << currentDevice;
            break;
        }
        case NetworkManager::Device::Ethernet: {
            currentDevice.reset(new NetworkManager::WiredDevice(uni));
            m_deviceList << currentDevice;
            break;
        }
        default:
            break;
        }
    }

#ifdef USE_DEEPIN_NMQT
    connect(currentDevice.get(), &NetworkManager::Device::interfaceFlagsChanged, currentDevice.get(), [ currentDevice, deviceCreateOrRemove ] {
        deviceCreateOrRemove(currentDevice);
    });
#endif
    connect(currentDevice.get(), &NetworkManager::Device::managedChanged, currentDevice.get(), [ currentDevice, deviceCreateOrRemove ] {
        deviceCreateOrRemove(currentDevice);
    });

    if (currentDevice->managed()
#ifdef USE_DEEPIN_NMQT
                 && ((currentDevice->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP) ||
                     currentDevice->type() == NetworkManager::Device::Wifi)
#endif
            ) {
        NetworkDeviceBase *newDevice = createDevice(currentDevice);
        connect(newDevice, &NetworkDeviceBase::deviceStatusChanged, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
        connect(newDevice, &NetworkDeviceBase::activeConnectionChanged, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
        connect(newDevice, &NetworkDeviceBase::ipV4Changed, this, &NetworkManagerProcesser::onUpdateNetworkDetail);
        m_devices << newDevice;
        sortDevice();
        updateDeviceName();
        onUpdateNetworkDetail();
        Q_EMIT deviceAdded({ newDevice });
        updateHotspot();
    }
}

void NetworkManagerProcesser::onDeviceRemove(const QString &uni)
{
    NetworkDeviceBase *rmDevice = Q_NULLPTR;
    for (NetworkDeviceBase *device : m_devices) {
        if (device->path() == uni) {
            m_devices.removeOne(device);
            rmDevice = device;
            break;
        }
    }

    QList<NetworkManager::Device::Ptr>::iterator itDevice = std::find_if(m_deviceList.begin(), m_deviceList.end(), [ uni ](NetworkManager::Device::Ptr device) {
        return (device->uni() == uni);
    });
    if (itDevice != m_deviceList.end()) {
        m_deviceList.removeAll(*itDevice);
    }

    if (rmDevice) {
        Q_EMIT rmDevice->removed();
        sortDevice();
        updateDeviceName();
        onUpdateNetworkDetail();
        Q_EMIT deviceRemoved({ rmDevice });
        rmDevice->deleteLater();
    }
}

void NetworkManagerProcesser::checkConnectivityFinished(quint32 conntity)
{
    onConnectivityChanged(NetworkManager::Connectivity(conntity));
}

void NetworkManagerProcesser::onConnectivityChanged(NetworkManager::Connectivity conntity)
{
    dde::network::Connectivity ctity;
    switch (conntity) {
    case NetworkManager::Connectivity::Full: {
        ctity = dde::network::Connectivity::Full;
        break;
    }
    case NetworkManager::Connectivity::Portal: {
        ctity = dde::network::Connectivity::Portal;
        break;
    }
    case NetworkManager::Connectivity::Limited: {
        ctity = dde::network::Connectivity::Limited;
        break;
    }
    case NetworkManager::Connectivity::NoConnectivity: {
        ctity = dde::network::Connectivity::Noconnectivity;
        break;
    }
    case NetworkManager::Connectivity::UnknownConnectivity: {
        ctity = dde::network::Connectivity::Unknownconnectivity;
        break;
    }
    }
    if (m_connectivity != ctity) {
        m_connectivity = ctity;
        Q_EMIT connectivityChanged(m_connectivity);
    }
}

// 虚接口
ProcesserInterface::ProcesserInterface()
{
}

ProcesserInterface::~ProcesserInterface()
{
}
