// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ipconflicthandler.h"
#include "settingconfig.h"
#include "utils/constants.h"

#include <QDBusConnection>
#include <QDebug>

#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

const QString ipwatchdService = "com.lingmo.system.IPWatchD";
const QString ipwatchdPath = "/com/lingmo/system/IPWatchD";
const QString ipwatchdInterface = "com.lingmo.system.IPWatchD";

IPConflictHandler::IPConflictHandler(QObject *parent)
    : QObject (parent)
{
    initConnection();
    initDevice();
}

NetworkManager::Setting::SecretFlags IPConflictHandler::secretFlags(const NetworkManager::ActiveConnection::Ptr &activeConnection) const
{
    if (activeConnection.isNull())
        return NetworkManager::Setting::SecretFlagType::None;

    NetworkManager::WirelessSecuritySetting::Ptr wirelessSecretSetting = activeConnection->connection()->settings()
            ->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (wirelessSecretSetting) {
        // init KeyMgmt
        const NetworkManager::WirelessSecuritySetting::KeyMgmt &keyMgmt = wirelessSecretSetting->keyMgmt();
        // 当前选择的加密方式
        NetworkManager::WirelessSecuritySetting::KeyMgmt currentKeyMgmt = (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Unknown) ?
                           NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone : keyMgmt;

        // 当前选择的保密模式
        if (currentKeyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep) {
            return wirelessSecretSetting->wepKeyFlags();
        }
        if (currentKeyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk ||
                    currentKeyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaSae) {
            return wirelessSecretSetting->pskFlags();
        }
    } else {
        NetworkManager::Security8021xSetting::Ptr secretSetting = activeConnection->connection()->settings()
                ->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

        if (secretSetting) {
            const QList<NetworkManager::Security8021xSetting::EapMethod> &eapMethods = secretSetting->eapMethods();
            NetworkManager::Security8021xSetting::EapMethod method = eapMethods.isEmpty() ? NetworkManager::Security8021xSetting::EapMethodTls : eapMethods.first();
            return (method == NetworkManager::Security8021xSetting::EapMethodTls ?
                                                     secretSetting->privateKeyPasswordFlags() : secretSetting->passwordFlags());
        }
    }

    // 默认是为所有用户保存密码
    return NetworkManager::Setting::SecretFlagType::None;
}

void IPConflictHandler::requestReConnect(const QSharedPointer<NetworkManager::Device> &device)
{
    if (device.isNull()) {
        qCWarning(DSM) << "cannot get device.";
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();

    if (canReconnect(activeConnection)) {
        QString activePath = activeConnection->path();
        QString activeConnectionPath = activeConnection->connection()->path();
        QDBusPendingReply<> reply = NetworkManager::deactivateConnection(activePath);
        reply.waitForFinished();
        NetworkManager::activateConnection(activeConnectionPath, device->uni(), "");
    }
}

bool IPConflictHandler::canReconnect(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection)
{
    return (!activeConnection.isNull());
}

bool IPConflictHandler::ipConflicted(const QString &devicePath, const QString &ip) const
{
    return false;
}

void IPConflictHandler::initConnection()
{
    QDBusConnection::systemBus().connect(ipwatchdService, ipwatchdPath, ipwatchdInterface, "IPConflict", this, SLOT(onReceiveIPConflict(QString, QString, QString)));
    QDBusConnection::systemBus().connect(ipwatchdService, ipwatchdPath, ipwatchdInterface, "IPConflictReslove", this, SLOT(onReceiveIPConflictReslove(QString, QString, QString)));
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAoceand, this, [this] (const QString &uni) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
        if (device) {
            initDeviceConnection(device);
        }
    });
}

void IPConflictHandler::initDevice()
{
    // if ip changed, then I think it Reslove conflict
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        initDeviceConnection(device);
    }
}

void IPConflictHandler::initDeviceConnection(const NetworkManager::Device::Ptr &device)
{
    connect(device.data(), &NetworkManager::Device::ipV4ConfigChanged, this, [ this, device ] { ipv4Changed(device); });
    connect(device.data(), &NetworkManager::Device::ipV4AddressChanged, this, [ this, device ] { ipv4Changed(device); });
}

void IPConflictHandler::onReceiveIPConflict(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    onIPConflicted(ip, localMac, remoteMac);
}

void IPConflictHandler::onReceiveIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    onIPConflictReslove(ip, localMac, remoteMac);
}

/**
 * @brief IP冲突的数据存储
 */

IPConflictStore::IPConflictStore()
{
}

IPConflictStore::~IPConflictStore()
{
}

bool IPConflictStore::doConfilcted(const QString &ip, const QString &localMac, const QString &remoteMac, bool &needReconnect)
{
    needReconnect = false;
    QString macAddress = formatMac(localMac);
    NetworkManager::Device::Ptr device = getDevicePathByMac(macAddress);
    if (device.isNull()) {
        qCWarning(DSM) << "not found device by mac address";
        return false;
    }

    qCInfo(DSM) << "ip conflicted, ip:" << ip << "mac:" << macAddress << "remote mac:" << formatMac(remoteMac);
    if (m_conflictData.contains(macAddress)) {
        const QList<DeviceIpData> &datas = m_conflictData[macAddress];
        for (const DeviceIpData &data : datas) {
            if (data.m_ip == ip && data.m_localMac == macAddress && data.m_remoteMac == formatMac(remoteMac))
                return false;
        }
    } else if (SettingConfig::instance()->reconnectIfIpConflicted()) {
        // 如果是第一次IP冲突，则重新获取IP地址
        needReconnect = true;
    }
    DeviceIpData data;
    data.m_ip = ip;
    data.m_uni = device->uni();
    data.m_localMac = macAddress;
    data.m_remoteMac = formatMac(remoteMac);
    m_conflictData[macAddress] << data;
    return true;
}

bool IPConflictStore::doConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    QString macAddress = formatMac(localMac);
    if (!m_conflictData.contains(macAddress))
        return true;

    // 删除当前设备对应的解除IP
    int removeIndex = -1;
    QString deviceUni;
    QList<DeviceIpData> &datas = m_conflictData[macAddress];
    for (int i = 0; i < datas.size(); i++) {
        DeviceIpData data = datas[i];
        if (data.m_ip == ip && data.m_localMac == macAddress && data.m_remoteMac == formatMac(remoteMac)) {
            removeIndex = i;
            deviceUni = data.m_uni;
            break;
        }
    }

    if (removeIndex >= 0) {
        datas.removeAt(removeIndex);
        if (datas.isEmpty()) {
            m_conflictData.remove(macAddress);
            return true;
        }
    }

    return false;
}

bool IPConflictStore::isConflicted(const QString &devicePath) const
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(devicePath);
    if (device.isNull())
        return false;

    if (device->type() == NetworkManager::Device::Ethernet) {
        NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
        return m_conflictData.contains(wiredDevice->permanentHardwareAddress())
                || m_conflictData.contains(wiredDevice->hardwareAddress());
    }
    if (device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
        return m_conflictData.contains(wirelessDevice->permanentHardwareAddress())
                || m_conflictData.contains(wirelessDevice->hardwareAddress());
    }

    return false;
}

QString IPConflictStore::formatMac(const QString &macAddress) const
{
    QStringList macs = macAddress.split(":");
    for (QString &mac : macs) {
        mac = mac.toUpper();
        if (mac.length() == 1)
            mac = QString("0%1").arg(mac);
    }

    return macs.join(":");
}

bool IPConflictStore::isConflicted(const QString &devicePath, const QString &ip) const
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(devicePath);
    if (device.isNull())
        return false;

    QList<DeviceIpData> conflictedIps;
    if (device->type() == NetworkManager::Device::Ethernet) {
        NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
        if (m_conflictData.contains(wiredDevice->permanentHardwareAddress())) {
            conflictedIps = m_conflictData[wiredDevice->permanentHardwareAddress()];
        } else if (m_conflictData.contains(wiredDevice->hardwareAddress())) {
            conflictedIps = m_conflictData[wiredDevice->hardwareAddress()];
        }
    }
    if (device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
        if (m_conflictData.contains(wirelessDevice->permanentHardwareAddress())) {
            conflictedIps = m_conflictData[wirelessDevice->permanentHardwareAddress()];
        } else if (m_conflictData.contains(wirelessDevice->hardwareAddress())) {
            conflictedIps = m_conflictData[wirelessDevice->hardwareAddress()];
        }
    }

    for (const DeviceIpData &data : conflictedIps) {
        if (data.m_ip == ip) {
            return true;
        }
    }

    return false;
}

QSharedPointer<NetworkManager::Device> IPConflictStore::getDevicePathByMac(const QString &mac) const
{
    QString permanentMac = formatMac(mac);
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    auto itDevice = std::find_if(devices.begin(), devices.end(), [ permanentMac ](NetworkManager::Device::Ptr device) {
         if (device->type() == NetworkManager::Device::Type::Ethernet) {
            NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
            // permanentHardwareAddress 是设备的永久地址(出厂地址，不会发生变化)
            // hardwareAddress是设备的Mac地址，用户可以更改，IP冲突拿到的MAC地址是该地址
            // 因此，需要使用设备的MAC地址进行判断，不过默认情况下，MAC地址和永久地址相同
            return (wiredDevice->permanentHardwareAddress() == permanentMac
                    || wiredDevice->hardwareAddress() == permanentMac);
         }

         if (device->type() == NetworkManager::Device::Type::Wifi) {
            NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
            return (wirelessDevice->permanentHardwareAddress() == permanentMac
                    || wirelessDevice->hardwareAddress() == permanentMac);
         }

         return false;
    });

    if (itDevice == devices.end())
        return nullptr;

    return (*itDevice);
}

void IPConflictStore::updateIpv4(const NetworkManager::Device::Ptr &device, const QStringList &ips, QStringList &resloveIps)
{
    QString macAddress;
    if (device->type() == NetworkManager::Device::Ethernet) {
        NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
        if (m_conflictData.contains(wiredDevice->permanentHardwareAddress())) {
            macAddress = wiredDevice->permanentHardwareAddress();
        } else if (m_conflictData.contains(wiredDevice->hardwareAddress())) {
            macAddress = wiredDevice->hardwareAddress();
        }
    } else if (device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
        if (m_conflictData.contains(wirelessDevice->permanentHardwareAddress())) {
            macAddress = wirelessDevice->permanentHardwareAddress();
        } else if (m_conflictData.contains(wirelessDevice->hardwareAddress())) {
            macAddress = wirelessDevice->hardwareAddress();
        }
    }

    if (macAddress.isEmpty() || !m_conflictData.contains(macAddress))
        return;

    qCDebug(DSM) << device->interfaceName() << device->uni() << "ip changed, reslove ip conflicted";
    QList<DeviceIpData> resloveData;
    QList<DeviceIpData> &conflictedData = m_conflictData[macAddress];
    for (const DeviceIpData &conflictData : conflictedData) {
        if (ips.contains(conflictData.m_ip))
            continue;

        resloveIps << conflictData.m_ip;
        resloveData << conflictData;
        qCDebug(DSM) << "reslove ip conflicted ip" << conflictData.m_ip;
    }

    for (const DeviceIpData &conflictData : resloveData) {
        conflictedData.removeAll(conflictData);
    }
    if (conflictedData.isEmpty()) {
        m_conflictData.remove(macAddress);
    }
}
