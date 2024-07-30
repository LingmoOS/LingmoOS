/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DEVICE_P_H
#define NETWORKMANAGERQT_DEVICE_P_H

#include "dbus/deviceinterface.h"
#include "device.h"

namespace NetworkManager
{
class NetworkManagerPrivate;

class DevicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit DevicePrivate(const QString &path, Device *q);
    ~DevicePrivate() override;

    void init();

    OrgFreedesktopNetworkManagerDeviceInterface deviceIface;
    Device::Capabilities capabilities;
    QString uni;
    QString udi;
    QString activeConnection;
    int designSpeed;
    Device::Type deviceType;
    Device::State connectionState;
    bool managed;
    mutable IpConfig ipV4Config;
    QString ipV4ConfigPath;
    mutable IpConfig ipV6Config;
    QString ipV6ConfigPath;
    QString driver;
    QHostAddress ipV4Address;
    QString interfaceName;
    QString ipInterface;
    Device::Interfaceflags interfaceFlags;
    bool firmwareMissing;
    mutable Dhcp4Config::Ptr dhcp4Config;
    QString dhcp4ConfigPath;
    mutable Dhcp6Config::Ptr dhcp6Config;
    QString dhcp6ConfigPath;
    QString driverVersion;
    QString firmwareVersion;
    QStringList availableConnections;
    bool autoconnect;
    Device::StateChangeReason reason;
    QString physicalPortId;
    uint mtu;
    bool nmPluginMissing;
    Device::MeteredStatus metered;
    DeviceStatistics::Ptr deviceStatistics;

    static NetworkManager::Device::MeteredStatus convertMeteredStatus(uint);
    static NetworkManager::Device::Capabilities convertCapabilities(uint);
    static NetworkManager::Device::Interfaceflags convertInterfaceflags(uint);
    static NetworkManager::Device::State convertState(uint);
    static NetworkManager::Device::StateChangeReason convertReason(uint);
    static NetworkManager::Device::Type convertType(uint);

    Q_DECLARE_PUBLIC(Device)
    Device *q_ptr;

public:
    /**
     * When subclassing make sure to call the parent class method
     * if the property was not useful to your new class
     */
    virtual void propertyChanged(const QString &property, const QVariant &value);

public Q_SLOTS:
    void deviceStateChanged(uint, uint, uint);
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void propertiesChanged(const QVariantMap &properties);
};

} // namespace NetworkManager
#endif
