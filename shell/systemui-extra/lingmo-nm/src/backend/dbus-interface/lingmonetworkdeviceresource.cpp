/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lingmonetworkdeviceresource.h"
#include "kywirelessnetitem.h"
#include "lingmoutil.h"

#define VIRTURAL_DEVICE_PATH "/sys/devices/virtual/net"
#define LOG_FLAG "KyNetworkDeviceResourse"

KyNetworkDeviceResourse::KyNetworkDeviceResourse(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");
    qRegisterMetaType<NetworkManager::Connectivity>("NetworkManager::Connectivity");
    m_networkResourceInstance = KyNetworkResourceManager::getInstance();

    m_deviceMap.clear();

    initDeviceMap();

    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceAdd,
                                       this, &KyNetworkDeviceResourse::onDeviceAdd/*, Qt::ConnectionType::DirectConnection*/);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceRemove,
                                       this, &KyNetworkDeviceResourse::onDeviceRemove/*, Qt::ConnectionType::DirectConnection*/);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceUpdate,
                                       this, &KyNetworkDeviceResourse::onDeviceUpdate/*, Qt::ConnectionType::DirectConnection*/);

    connect(m_networkResourceInstance, &KyNetworkResourceManager::stateChanged,
                                       this, &KyNetworkDeviceResourse::stateChanged/*, Qt::ConnectionType::DirectConnection*/);

    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceCarrierChanage,
                                       this, &KyNetworkDeviceResourse::carrierChanage);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceBitRateChanage,
                                       this, &KyNetworkDeviceResourse::deviceBitRateChanage);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceMacAddressChanage,
                                       this, &KyNetworkDeviceResourse::deviceMacAddressChanage);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceActiveChanage,
                                       this, &KyNetworkDeviceResourse::deviceActiveChanage);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::deviceManagedChange,
                                       this, &KyNetworkDeviceResourse::deviceManagedChange);
}

KyNetworkDeviceResourse::~KyNetworkDeviceResourse()
{
    m_networkResourceInstance = nullptr;
}

void KyNetworkDeviceResourse::initDeviceMap()
{
    NetworkManager::Device::List deviceList
            = m_networkResourceInstance->getNetworkDeviceList();

    if (deviceList.isEmpty()) {
        qDebug() << LOG_FLAG << "there is not interface in computer.";
        return;
    }

    NetworkManager::Device::Ptr devicePtr = nullptr;
    for (int index = 0; index < deviceList.size(); ++index) {
        devicePtr = deviceList.at(index);
        if (devicePtr.isNull()) {
            continue;
        }

        m_deviceMap.insert(devicePtr->uni(), devicePtr->interfaceName());
    }

    return;
}

void KyNetworkDeviceResourse::getNetworkDeviceList(
                NetworkManager::Device::Type deviceType,
                QStringList &networkDeviceList)
{
    NetworkManager::Device::List deviceList
            = m_networkResourceInstance->getNetworkDeviceList();

    if (deviceList.isEmpty()) {
        qDebug() << LOG_FLAG <<"network device is not exist. device type" << deviceType;
        return;
    }

    NetworkManager::Device::Ptr devicePtr = nullptr;
    for (int index = 0; index < deviceList.size(); ++index) {
        devicePtr = deviceList.at(index);
        if (devicePtr.isNull()) {
            continue;
        }

        if (devicePtr->type() == deviceType) {
            if (NetworkManager::Device::Type::Ethernet == deviceType) {
                //为了区分有线网卡和虚拟网卡
                qDebug()<< LOG_FLAG << "device uni" << devicePtr->udi();
                if (devicePtr->udi().startsWith(VIRTURAL_DEVICE_PATH)) {
                    continue;
                }
            }

            networkDeviceList<<devicePtr->interfaceName();
        }
    }

    return;
}

void KyNetworkDeviceResourse::getHardwareInfo(QString ifaceName, QString &hardAddress, int &bandWith)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->getNetworkDevice(ifaceName);

    if (nullptr == connectDevice || !connectDevice->isValid()) {
        qWarning()<< LOG_FLAG <<"get hardware info failed, the device" << ifaceName << "is not existed";
        hardAddress.clear();
        bandWith = 0;
        return;
    }

    switch (connectDevice->type()) {
        case NetworkManager::Device::Ethernet:
        {
            NetworkManager::WiredDevice *wiredDevicePtr =
                qobject_cast<NetworkManager::WiredDevice *>(connectDevice.data());
            hardAddress = wiredDevicePtr->hardwareAddress();
            bandWith = wiredDevicePtr->bitRate();
            break;
        }
        case NetworkManager::Device::Wifi:
        {
            NetworkManager::WirelessDevice *wirelessDevicePtr =
                qobject_cast<NetworkManager::WirelessDevice *>(connectDevice.data());
            hardAddress = wirelessDevicePtr->hardwareAddress();
            bandWith = wirelessDevicePtr->bitRate();
            break;
        }
        default:
        {
            hardAddress = "";
            bandWith = 0;
            qWarning()<< LOG_FLAG << "the network device type is undefined" << connectDevice->type();
            break;
        }
    }

    return;
}

NetworkManager::Device::State KyNetworkDeviceResourse::getDeviceState(QString deviceName)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (!connectDevice.isNull() && connectDevice->isValid()) {
        return connectDevice->state();
    }

    qWarning()<< LOG_FLAG <<"get device state failed, the device is " << deviceName;

    return NetworkManager::Device::State::UnknownState;
}

bool KyNetworkDeviceResourse::wiredDeviceIsCarriered(QString deviceName)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice.isNull()) {
        qDebug()<< LOG_FLAG << "check device carriered failed.";
        return false;
    }


    if (connectDevice->isValid()
            && NetworkManager::Device::Type::Ethernet == connectDevice->type()) {
        NetworkManager::WiredDevice *wiredDevicePtr =
            qobject_cast<NetworkManager::WiredDevice *>(connectDevice.data());
        return wiredDevicePtr->carrier();
    }

    qWarning()<< LOG_FLAG << deviceName <<" can not get carrier state.";

    return false;
}

void KyNetworkDeviceResourse::setDeviceRefreshRate(QString deviceName, int ms)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice.isNull()) {
        return;
    }

    if (connectDevice->isValid()) {
        NetworkManager::DeviceStatistics::Ptr deviceStatistics = connectDevice->deviceStatistics();
        deviceStatistics->setRefreshRateMs(ms);
    }

    return;
}

qulonglong KyNetworkDeviceResourse::getDeviceRxRefreshRate(QString deviceName)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice->isValid()) {
        NetworkManager::DeviceStatistics::Ptr deviceStatistics = connectDevice->deviceStatistics();
        qulonglong rx = 0;
        rx = deviceStatistics->rxBytes();
        if (rx != 0) {
            return rx;
        } else {
            qDebug() << "connectDevice is invalid we do not get rxrate";
        }
    }

    return 0;
}

qulonglong KyNetworkDeviceResourse::getDeviceTxRefreshRate(QString deviceName)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice->isValid()) {
        NetworkManager::DeviceStatistics::Ptr deviceStatistics = connectDevice->deviceStatistics();
        qulonglong tx = 0;
        tx = deviceStatistics->txBytes();
        if (tx != 0){
            return tx;
        } else {
        qDebug() << "connectDevice is invalid we do not get txrate";
        }
    }

    return 0;
}

void KyNetworkDeviceResourse::getDeviceConnectivity(const QString &deviceName, NetworkManager::Connectivity &connectivity)
{
    connectivity = NetworkManager::Connectivity::UnknownConnectivity;
    QString dbusPath;
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice != nullptr && connectDevice->isValid()) {
       dbusPath = connectDevice->uni();
    } else {
        qWarning() << "[KyNetworkDeviceResourse] can not find device " << deviceName;
        return;
    }

    QDBusInterface *ip4ConnectivityDbus = new QDBusInterface("org.freedesktop.NetworkManager",
                                                             dbusPath,
                                                             "org.freedesktop.DBus.Properties",
                                                             QDBusConnection::systemBus());

    if (ip4ConnectivityDbus == nullptr || !ip4ConnectivityDbus->isValid()) {
        qWarning() << "[KyNetworkDeviceResourse] get device properties failed";
        return;
    }

    QDBusReply<QVariant> reply = ip4ConnectivityDbus->call("Get", "org.freedesktop.NetworkManager.Device", "Ip4Connectivity");

    if (reply.isValid()) {
        connectivity = (NetworkManager::Connectivity) reply.value().toUInt();
    } else {
        qWarning() << "[KyNetworkDeviceResourse] get device properties failed";
    }

    delete ip4ConnectivityDbus;
    ip4ConnectivityDbus = nullptr;
}

bool KyNetworkDeviceResourse::getActiveConnectionInfo(const QString devName, int &signalStrength, QString &uni, QString &secuType)
{
    signalStrength = 0;
    uni = "";
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->getNetworkDevice(devName);

    if (nullptr == connectDevice || !connectDevice->isValid()) {
        qWarning()<< LOG_FLAG <<"getDeviceActiveAPInfo failed, the device" << devName << "is not existed";
        return false;
    }

    if (connectDevice->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice *wirelessDevicePtr =
            qobject_cast<NetworkManager::WirelessDevice *>(connectDevice.data());
        NetworkManager::AccessPoint::Ptr apPtr = wirelessDevicePtr->activeAccessPoint();
        if (apPtr.isNull()) {
            return false;
        }
        signalStrength = apPtr->signalStrength();
        uni = apPtr->uni();
        NetworkManager::AccessPoint::Capabilities cap = apPtr->capabilities();
        NetworkManager::AccessPoint::WpaFlags wpaFlag = apPtr->wpaFlags();
        NetworkManager::AccessPoint::WpaFlags rsnFlag = apPtr->rsnFlags();
        secuType = enumToQstring(cap, wpaFlag, rsnFlag);
        return true;
    } else {
        return false;
    }
}

const QMap<uint, uint> g_bFreqs = {
    {2412, 1},
    {2417, 2},
    {2422, 3},
    {2427, 4},
    {2432, 5},
    {2437, 6},
    {2442, 7},
    {2447, 8},
    {2452, 9},
    {2457, 10},
    {2462, 11},
    {2467, 12},
    {2472, 13},
    {2484, 14}
};

const QMap<uint, uint> g_aFreqs = {
    {5035, 7},
    {5040, 8},
    {5045, 9},
    {5055, 11},
    {5060, 12},
    {5080, 16},
    {5170, 34},
    {5180, 36},
    {5190, 38},
    {5200, 40},
    {5210, 42},
    {5220, 44},
    {5230, 46},
    {5240, 48},
    {5260, 52},
    {5280, 56},
    {5300, 60},
    {5320, 64},
    {5500, 100},
    {5520, 104},
    {5540, 108},
    {5560, 112},
    {5580, 116},
    {5600, 120},
    {5620, 124},
    {5640, 128},
    {5660, 132},
    {5680, 136},
    {5700, 140},
    {5745, 149},
    {5765, 153},
    {5785, 157},
    {5805, 161},
    {5825, 165},
    {4915, 183},
    {4920, 184},
    {4925, 185},
    {4935, 187},
    {4940, 188},
    {4945, 189},
    {4960, 192},
    {4980, 196}
};

uint KyNetworkDeviceResourse::kyFindChannel(uint freq)
{
    uint channel = 0;
    QMap<uint, uint> freqMap;
    if (freq < 2500) {
        freqMap = g_bFreqs;
    } else {
        freqMap = g_aFreqs;
    }
    for (auto freqKey : freqMap.keys()) {
        if (freqKey <= freq) {
            channel = freqMap.value(freqKey);
            if (freqKey == freq) {
                break;
            }
        } else {
            break;
        }
    }

    return channel;
}

void KyNetworkDeviceResourse::getDeviceActiveAPInfo(const QString devName, QString &strMac, uint &iHz, uint &iChan, QString &secuType)
{
    strMac.clear();
    iHz = 0;
    iChan = 0;
    secuType.clear();

    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->getNetworkDevice(devName);

    if (nullptr == connectDevice || !connectDevice->isValid()) {
        qWarning()<< LOG_FLAG <<"getDeviceActiveAPInfo failed, the device" << devName << "is not existed";
        return;
    }

    switch (connectDevice->type()) {
        case NetworkManager::Device::Wifi:
        {
            NetworkManager::WirelessDevice *wirelessDevicePtr =
                qobject_cast<NetworkManager::WirelessDevice *>(connectDevice.data());
            NetworkManager::AccessPoint::Ptr apPtr = wirelessDevicePtr->activeAccessPoint();
            if (apPtr.isNull()) {
                break;
            }
            strMac = apPtr->hardwareAddress();
            iHz = apPtr->frequency();
            iChan = kyFindChannel(iHz);
            NetworkManager::AccessPoint::Capabilities cap = apPtr->capabilities();
            NetworkManager::AccessPoint::WpaFlags wpaFlag = apPtr->wpaFlags();
            NetworkManager::AccessPoint::WpaFlags rsnFlag = apPtr->rsnFlags();
            secuType = enumToQstring(cap, wpaFlag, rsnFlag);
            break;
        }
        case NetworkManager::Device::Ethernet:
            break;
        default:
            break;
    }

    return;
}

int KyNetworkDeviceResourse::getWirelessDeviceCapability(const QString deviceName)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice->isValid()
            && NetworkManager::Device::Type::Wifi == connectDevice->type()) {
        NetworkManager::WirelessDevice *wirelessDevicePtr =
            qobject_cast<NetworkManager::WirelessDevice *>(connectDevice.data());

        int cap = 0x00;
        if (wirelessDevicePtr->wirelessCapabilities() & NetworkManager::WirelessDevice::ApCap) {
            cap = cap | 0x02;
        } else {
            return 0x01;
        }
        QDBusInterface dbusInterface("org.freedesktop.NetworkManager",
                                     connectDevice->uni(),
                                     "org.freedesktop.NetworkManager.Device.Wireless",
                                     QDBusConnection::systemBus());

        QDBusReply<uint> reply = dbusInterface.call("GetHotspotCapabilities");
        if (reply.isValid()) {
            if (reply.value() == 1) {
                cap = cap | 0x04;
            }
        }
        return cap;
    } else {
        qWarning()<<"[KyNetworkDeviceResourse]"<<deviceName<<" is not valid or not wireless.";
    }
    return 0x01;
}

void KyNetworkDeviceResourse::onDeviceAdd(QString deviceName, QString uni, NetworkManager::Device::Type deviceType)
{
    m_deviceMap.insert(uni, deviceName);
    Q_EMIT deviceAdd(deviceName, deviceType);
    return;
}

void KyNetworkDeviceResourse::onDeviceRemove(QString deviceName, QString uni)
{
    m_deviceMap.remove(uni);
    Q_EMIT deviceRemove(deviceName);
    return;
}

void KyNetworkDeviceResourse::onDeviceUpdate(QString interface, QString dbusPath)
{
    if (m_deviceMap.contains(dbusPath)) {
        if (m_deviceMap[dbusPath] != interface) {
            QString oldName = m_deviceMap[dbusPath];
            m_deviceMap[dbusPath] = interface;
            Q_EMIT deviceNameUpdate(oldName, interface);
        }
    }

    return;
}

bool KyNetworkDeviceResourse::wirelessDeviceIsExist(const QString devName)
{
    QStringList list;
    getNetworkDeviceList(NetworkManager::Device::Type::Wifi, list);
    return list.contains(devName);
}

bool KyNetworkDeviceResourse::checkDeviceType(QString deviceName, NetworkManager::Device::Type deviceType)
{
    NetworkManager::Device::Ptr devicePtr =
                m_networkResourceInstance->findDeviceInterface(deviceName);

    if (devicePtr.isNull()) {
        qDebug() << LOG_FLAG << "check device type failed, it is not exist";
        return false;
    }

    if (deviceType == devicePtr->type()) {
        if (NetworkManager::Device::Type::Ethernet == deviceType) {
            if (!devicePtr->udi().startsWith(VIRTURAL_DEVICE_PATH)) {
                return true;
            }
        } else {
            return true;
        }
    }

    return false;
}

void KyNetworkDeviceResourse::setDeviceManaged(QString devName, bool managed)
{
    QString dbusPath;
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(devName);
    if (connectDevice->isValid()) {
       dbusPath = connectDevice->uni();
    } else {
        qWarning()<<"[KyNetworkDeviceResourse] can not find device " << devName;
        return;
    }
    setDeviceManagedByGDbus(dbusPath, managed);
#if 0
    QDBusInterface dbusInterface("org.freedesktop.NetworkManager",
                              dbusPath,
                              "org.freedesktop.NetworkManager.Device",
                              QDBusConnection::systemBus());

    if (!dbusInterface.isValid()) {
        qWarning() << dbusPath << "invalid";
        setDeviceManagedByGDbus(dbusPath, managed);
        return;
    }

    QDBusReply<void> reply = dbusInterface.call("SetStateDevice", "", managed);
    if (!reply.isValid()) {
        qWarning() << "SetStateDevice error" << reply.error().message();
        setDeviceManagedByGDbus(dbusPath, managed);
        return;
    }
#endif
}

bool KyNetworkDeviceResourse::getDeviceManaged(QString deviceName)
{
    NetworkManager::Device::Ptr connectDevice =
                        m_networkResourceInstance->findDeviceInterface(deviceName);
    if (connectDevice != nullptr && connectDevice->isValid()) {
       return connectDevice->managed();
    } else {
        qWarning()<<"[KyNetworkDeviceResourse] can not find device " << deviceName;
        return false;
    }
}
